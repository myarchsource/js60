// |reftest| skip -- regexp-unicode-property-escapes is not supported
// Copyright 2017 Mathias Bynens. All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
author: Mathias Bynens
description: >
  Unicode property escapes for `Script_Extensions=Kaithi`
info: |
  Generated by https://github.com/mathiasbynens/unicode-property-escapes-tests
  Unicode v10.0.0
  Emoji v5.0 (UTR51)
esid: sec-static-semantics-unicodematchproperty-p
features: [regexp-unicode-property-escapes]
includes: [regExpUtils.js]
---*/

const matchSymbols = buildString({
  loneCodePoints: [],
  ranges: [
    [0x000966, 0x00096F],
    [0x00A830, 0x00A839],
    [0x011080, 0x0110C1]
  ]
});
testPropertyEscapes(
  /^\p{Script_Extensions=Kaithi}+$/u,
  matchSymbols,
  "\\p{Script_Extensions=Kaithi}"
);
testPropertyEscapes(
  /^\p{Script_Extensions=Kthi}+$/u,
  matchSymbols,
  "\\p{Script_Extensions=Kthi}"
);
testPropertyEscapes(
  /^\p{scx=Kaithi}+$/u,
  matchSymbols,
  "\\p{scx=Kaithi}"
);
testPropertyEscapes(
  /^\p{scx=Kthi}+$/u,
  matchSymbols,
  "\\p{scx=Kthi}"
);

const nonMatchSymbols = buildString({
  loneCodePoints: [],
  ranges: [
    [0x00DC00, 0x00DFFF],
    [0x000000, 0x000965],
    [0x000970, 0x00A82F],
    [0x00A83A, 0x00DBFF],
    [0x00E000, 0x01107F],
    [0x0110C2, 0x10FFFF]
  ]
});
testPropertyEscapes(
  /^\P{Script_Extensions=Kaithi}+$/u,
  nonMatchSymbols,
  "\\P{Script_Extensions=Kaithi}"
);
testPropertyEscapes(
  /^\P{Script_Extensions=Kthi}+$/u,
  nonMatchSymbols,
  "\\P{Script_Extensions=Kthi}"
);
testPropertyEscapes(
  /^\P{scx=Kaithi}+$/u,
  nonMatchSymbols,
  "\\P{scx=Kaithi}"
);
testPropertyEscapes(
  /^\P{scx=Kthi}+$/u,
  nonMatchSymbols,
  "\\P{scx=Kthi}"
);

reportCompare(0, 0);