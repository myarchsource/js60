/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=8 sts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "ClientValidation.h"

#include "ClientPrefs.h"
#include "mozilla/net/MozURL.h"

namespace mozilla {
namespace dom {

using mozilla::ipc::ContentPrincipalInfo;
using mozilla::ipc::PrincipalInfo;
using mozilla::net::MozURL;

bool ClientIsValidPrincipalInfo(const PrincipalInfo& aPrincipalInfo) {
  // Ideally we would verify that the source process has permission to
  // create a window or worker with the given principal, but we don't
  // currently have any such restriction in place.  Instead, at least
  // verify the PrincipalInfo is an expected type and has a parsable
  // origin/spec.
  switch (aPrincipalInfo.type()) {
    // Any system and null principal is acceptable.
    case PrincipalInfo::TSystemPrincipalInfo:
    case PrincipalInfo::TNullPrincipalInfo: {
      return true;
    }

    // Validate content principals to ensure that the origin and spec are sane.
    case PrincipalInfo::TContentPrincipalInfo: {
      const ContentPrincipalInfo& content =
          aPrincipalInfo.get_ContentPrincipalInfo();

      // Verify the principal spec parses.
      RefPtr<MozURL> specURL;
      nsresult rv = MozURL::Init(getter_AddRefs(specURL), content.spec());
      NS_ENSURE_SUCCESS(rv, false);

      // Verify the principal originNoSuffix parses.
      RefPtr<MozURL> originURL;
      rv = MozURL::Init(getter_AddRefs(originURL), content.originNoSuffix());
      NS_ENSURE_SUCCESS(rv, false);

      nsAutoCString originOrigin;
      rv = originURL->GetOrigin(originOrigin);
      NS_ENSURE_SUCCESS(rv, false);

      nsAutoCString specOrigin;
      rv = specURL->GetOrigin(specOrigin);
      NS_ENSURE_SUCCESS(rv, false);

      // For now require Clients to have a principal where both its
      // originNoSuffix and spec have the same origin.  This will
      // exclude a variety of unusual combinations within the browser
      // but its adequate for the features need to support right now.
      // If necessary we could expand this function to handle more
      // cases in the future.
      return specOrigin == originOrigin;
    }
    default: { break; }
  }

  // Windows and workers should not have expanded URLs, etc.
  return false;
}

bool ClientIsValidCreationURL(const PrincipalInfo& aPrincipalInfo,
                              const nsACString& aURL) {
  RefPtr<MozURL> url;
  nsresult rv = MozURL::Init(getter_AddRefs(url), aURL);
  NS_ENSURE_SUCCESS(rv, false);

  switch (aPrincipalInfo.type()) {
    case PrincipalInfo::TContentPrincipalInfo: {
      // Any origin can create an about:blank or about:srcdoc Client.
      if (aURL.LowerCaseEqualsLiteral("about:blank") ||
          aURL.LowerCaseEqualsLiteral("about:srcdoc")) {
        return true;
      }

      const ContentPrincipalInfo& content =
          aPrincipalInfo.get_ContentPrincipalInfo();

      // Parse the principal origin URL as well.  This ensures any MozURL
      // parser issues effect both URLs equally.
      RefPtr<MozURL> principalURL;
      rv = MozURL::Init(getter_AddRefs(principalURL), content.originNoSuffix());
      NS_ENSURE_SUCCESS(rv, false);

      nsAutoCString origin;
      rv = url->GetOrigin(origin);
      NS_ENSURE_SUCCESS(rv, false);

      nsAutoCString principalOrigin;
      rv = principalURL->GetOrigin(principalOrigin);
      NS_ENSURE_SUCCESS(rv, false);

      // The vast majority of sites should simply result in the same principal
      // and URL origin.
      if (principalOrigin == origin) {
        return true;
      }

      nsAutoCString scheme;
      rv = url->GetScheme(scheme);
      NS_ENSURE_SUCCESS(rv, false);

      // Generally any origin can also open javascript: windows and workers.
      if (scheme.LowerCaseEqualsLiteral("javascript")) {
        return true;
      }

      // We have some tests that use data: URL windows without an opaque
      // origin.  This should only happen when a pref is set.
      if (!ClientPrefsGetDataURLUniqueOpaqueOrigin() &&
          scheme.LowerCaseEqualsLiteral("data")) {
        return true;
      }

      nsAutoCString principalScheme;
      rv = principalURL->GetScheme(principalScheme);
      NS_ENSURE_SUCCESS(rv, false);

      // Otherwise don't support this URL type in the clients sub-system for
      // now.  This will exclude a variety of internal browser clients, but
      // currently we don't need to support those.  This function can be
      // expanded to handle more cases as necessary.
      return false;
    }
    case PrincipalInfo::TSystemPrincipalInfo: {
      nsAutoCString scheme;
      rv = url->GetScheme(scheme);
      NS_ENSURE_SUCCESS(rv, false);

      // While many types of documents can be created with a system principal,
      // there are only a few that can reasonably become windows.  We attempt
      // to validate the list of known cases here with a simple scheme check.
      return scheme.LowerCaseEqualsLiteral("about") ||
             scheme.LowerCaseEqualsLiteral("chrome") ||
             scheme.LowerCaseEqualsLiteral("resource") ||
             scheme.LowerCaseEqualsLiteral("blob") ||
             scheme.LowerCaseEqualsLiteral("javascript") ||
             scheme.LowerCaseEqualsLiteral("view-source") ||

             (!ClientPrefsGetDataURLUniqueOpaqueOrigin() &&
              scheme.LowerCaseEqualsLiteral("data"));
    }
    case PrincipalInfo::TNullPrincipalInfo: {
      // A wide variety of clients can have a null principal.  For example,
      // sandboxed iframes can have a normal content URL.  For now allow
      // any parsable URL for null principals.  This is relatively safe since
      // null principals have unique origins and won't most ClientManagerService
      // queries anyway.
      return true;
    }
    default: { break; }
  }

  // Clients (windows/workers) should never have an expanded principal type.
  return false;
}

}  // namespace dom
}  // namespace mozilla