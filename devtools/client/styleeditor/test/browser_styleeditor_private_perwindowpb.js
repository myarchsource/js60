/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

"use strict";

// This test makes sure that the style editor does not store any
// content CSS files in the permanent cache when opened from PB mode.

const TEST_URL = "http://" + TEST_HOST + "/browser/devtools/client/" +
  "styleeditor/test/test_private.html";

add_task(function* () {
  info("Opening a new private window");
  let win = OpenBrowserWindow({private: true});
  yield waitForDelayedStartupFinished(win);

  info("Clearing the browser cache");
  Services.cache2.clear();

  let { toolbox, ui } = yield openStyleEditorForURL(TEST_URL, win);

  is(ui.editors.length, 1, "The style editor contains one sheet.");
  let editor = ui.editors[0];

  yield editor.getSourceEditor();
  yield checkDiskCacheFor(TEST_HOST);

  yield toolbox.destroy();

  let onUnload = new Promise(done => {
    win.addEventListener("unload", function listener(event) {
      if (event.target == win.document) {
        win.removeEventListener("unload", listener);
        done();
      }
    });
  });
  win.close();
  yield onUnload;
});

function checkDiskCacheFor(host) {
  let foundPrivateData = false;

  return new Promise(resolve => {
    Visitor.prototype = {
      onCacheStorageInfo: function (num) {
        info("disk storage contains " + num + " entries");
      },
      onCacheEntryInfo: function (uri) {
        let urispec = uri.asciiSpec;
        info(urispec);
        foundPrivateData |= urispec.includes(host);
      },
      onCacheEntryVisitCompleted: function () {
        is(foundPrivateData, false, "web content present in disk cache");
        resolve();
      }
    };
    function Visitor() {}

    let storage =
      Services.cache2.diskCacheStorage(Services.loadContextInfo.default, false);
    storage.asyncVisitStorage(new Visitor(),
      /* Do walk entries */
      true);
  });
}

function waitForDelayedStartupFinished(win) {
  return new Promise(resolve => {
    Services.obs.addObserver(function observer(subject, topic) {
      if (win == subject) {
        Services.obs.removeObserver(observer, topic);
        resolve();
      }
    }, "browser-delayed-startup-finished");
  });
}