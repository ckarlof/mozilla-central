/*
 * ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is Mozilla code.
 *
 * The Initial Developer of the Original Code is
 * Mozilla Foundation.
 * Portions created by the Initial Developer are Copyright (C) 2007
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *    Robert Sayre <sayrer@gmail.com> (original author)
 *    Alexander J. Vincent <ajvincent@gmail.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */
 
function run_test() {   
  var scope = {};
  Components.utils.import("resource://gre/modules/XPCOMUtils.jsm", scope);
  do_check_eq(typeof(scope.XPCOMUtils), "object");
  do_check_eq(typeof(scope.XPCOMUtils.generateFactory), "function");
  
  // try again on the global object
  do_check_eq(typeof(Components.utils.import), "function");
  Components.utils.import("resource://gre/modules/XPCOMUtils.jsm");
  do_check_eq(typeof(XPCOMUtils), "object");
  do_check_eq(typeof(XPCOMUtils.generateFactory), "function");
  
  // try on a new object
  var scope2 = {};
  Components.utils.import("resource://gre/modules/XPCOMUtils.jsm", scope2);
  do_check_eq(typeof(scope2.XPCOMUtils), "object");
  do_check_eq(typeof(scope2.XPCOMUtils.generateFactory), "function");
  
  do_check_true(scope2.XPCOMUtils == scope.XPCOMUtils);

  // make sure we throw when the second arg is bogus
  var didThrow = false;
  try {
      Components.utils.import("resource://gre/modules/XPCOMUtils.jsm", "wrong");
  } catch (ex) {
      print("ex: " + ex);
      didThrow = true;
  }
  do_check_true(didThrow);
 
  // try to create a component
  do_load_module("/js/src/xpconnect/tests/unit/component_import.js");
  const contractID = "@mozilla.org/tests/module-importer;";
  do_check_true((contractID + "1") in Components.classes);
  var foo = Components.classes[contractID + "1"]
                      .createInstance(Components.interfaces.nsIClassInfo);
  do_check_true(Boolean(foo));
  do_check_true(foo.contractID == contractID + "1");

  // try to create another component which doesn't directly implement QI
  do_check_true((contractID + "2") in Components.classes);
  var bar = Components.classes[contractID + "2"]
                      .createInstance(Components.interfaces.nsIClassInfo);
  do_check_true(Boolean(bar));
  do_check_true(bar.contractID == contractID + "2");
}
