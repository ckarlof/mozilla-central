/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * The contents of this file are subject to the Netscape Public License
 * Version 1.0 (the "NPL"); you may not use this file except in
 * compliance with the NPL.  You may obtain a copy of the NPL at
 * http://www.mozilla.org/NPL/
 *
 * Software distributed under the NPL is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the NPL
 * for the specific language governing rights and limitations under the
 * NPL.
 *
 * The Initial Developer of this code under the NPL is Netscape
 * Communications Corporation.  Portions created by Netscape are
 * Copyright (C) 1998 Netscape Communications Corporation.  All Rights
 * Reserved.
 */  

#include <ctype.h> 
#include <time.h>
#include <stdio.h>  
#include "nsScanner.h"
#include "nsToken.h" 
#include "nsHTMLTokens.h"
#include "nsParserTypes.h"
#include "prtypes.h"
#include "nsDebug.h"

//#define GESS_MACHINE
#ifdef GESS_MACHINE
#include "nsEntityEx.cpp"
#endif

static nsString     gIdentChars("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-");
static nsString     gAttrTextChars("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-%.");
static nsString     gAlphaChars("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");
static nsAutoString gDigits("0123456789");
static nsAutoString gWhitespace(" \t\b");
static nsAutoString gOperatorChars("/?.<>[]{}~^+=-!%&*(),|:");
static const char*  gUserdefined = "userdefined";

//debug error messages...
static const char* kNullScanner = "Error: Scanner is null.";

const PRInt32 kMAXNAMELEN=10;
struct StrToUnicodeStruct
{
  char    mName[kMAXNAMELEN+1];
  PRInt32 mValue;
};


  // KEEP THIS LIST SORTED!
  // NOTE: This names table is sorted in ascii collating order. If you
  // add a new entry, make sure you put it in the right spot otherwise
  // the binary search code above will break! 
static StrToUnicodeStruct gStrToUnicodeTable[] =
{
  {"AElig", 0x00c6},  {"AMP",   0x0026},  {"Aacute",0x00c1},  
  {"Acirc", 0x00c2},  {"Agrave",0x00c0},  {"Aring", 0x00c5},  
  {"Atilde",0x00c3},  {"Auml",  0x00c4},  {"COPY",  0x00a9},  
  {"Ccedil",0x00c7},  {"ETH",   0x00d0},  {"Eacute",0x00c9},
  {"Ecirc", 0x00ca},  {"Egrave",0x00c8},  {"Euml",  0x00cb},  
  {"GT",    0x003e},  {"Iacute",0x00cd},  {"Icirc", 0x00ce},  
  {"Igrave",0x00cc},  {"Iuml",  0x00cf},  {"LT",    0x003c},  
  {"Ntilde",0x00d1},  {"Oacute",0x00d3},  {"Ocirc", 0x00d4},
  {"Ograve",0x00d2},  {"Oslash",0x00d8},  {"Otilde",0x00d5},  
  {"Ouml",  0x00d6},  {"QUOT",  0x0022},  {"REG",   0x00ae},  
  {"THORN", 0x00de},  {"Uacute",0x00da},  {"Ucirc", 0x00db},  
  {"Ugrave",0x00d9},  {"Uuml",  0x00dc},  {"Yacute",0x00dd},
  {"aacute",0x00e1},  {"acirc", 0x00e2},  {"acute", 0x00b4},  
  {"aelig", 0x00e6},  {"agrave",0x00e0},  {"amp",   0x0026},  
  {"aring", 0x00e5},  {"atilde",0x00e3},  {"auml",  0x00e4},  
  {"brvbar",0x00a6},  {"ccedil",0x00e7},  {"cedil", 0x00b8},
  {"cent",  0x00a2},  {"copy",  0x00a9},  {"curren",0x00a4},  
  {"deg",   0x00b0},  {"divide",0x00f7},  {"eacute",0x00e9},  
  {"ecirc", 0x00ea},  {"egrave",0x00e8},  {"eth",   0x00f0},  
  {"euml",  0x00eb},  {"frac12",0x00bd},  {"frac14",0x00bc},
  {"frac34",0x00be},  {"gt",    0x003e},  {"iacute",0x00ed},  
  {"icirc", 0x00ee},  {"iexcl", 0x00a1},  {"igrave",0x00ec},  
  {"iquest",0x00bf},  {"iuml",  0x00ef},  {"laquo",  0x00ab},  
  {"lt",    0x003c},  {"macr",  0x00af},  {"micro",  0x00b5},
  {"middot",0x00b7},  {"nbsp",  0x00a0},  {"not",   0x00ac}, 
  {"ntilde",0x00f1},  {"oacute",0x00f3},  {"ocirc", 0x00f4},  
  {"ograve",0x00f2},  {"ordf",  0x00aa},  {"ordm",  0x00ba},  
  {"oslash",0x00f8},  {"otilde",0x00f5},  {"ouml",  0x00f6},
  {"para",  0x00b6},  {"plusmn",0x00b1},  {"pound", 0x00a3},  
  {"quot",  0x0022},  {"raquo", 0x00bb},  {"reg",   0x00ae},  
  {"sect",  0x00a7},  {"shy",   0x00ad},  {"sup1",  0x00b9},  
  {"sup2",  0x00b2},  {"sup3",  0x00b3},  {"szlig", 0x00df},
  {"thorn", 0x00fe},  {"times", 0x00d7},  {"uacute",0x00fa},  
  {"ucirc", 0x00fb},  {"ugrave",0x00f9},  {"uml",   0x00a8},  
  {"uuml",  0x00fc},  {"yacute",0x00fd},  {"yen",   0x00a5},  
  {"yuml",  0x00ff}
};


struct HTMLTagEntry {
  char      mName[12];
  eHTMLTags mTagID;
  PRInt16   mUnused;
};

  // KEEP THIS LIST SORTED!
  // NOTE: This table is sorted in ascii collating order. If you
  // add a new entry, make sure you put it in the right spot otherwise
  // the binary search code above will break! 
HTMLTagEntry gHTMLTagTable[] =
{
  {"!!UNKNOWN",   eHTMLTag_unknown,0},
  {"!DOCTYPE",    eHTMLTag_doctype,0},      {"A",         eHTMLTag_a,0},
  {"ACRONYM",     eHTMLTag_acronym,0},      {"ADDRESS",   eHTMLTag_address,0},
  {"APPLET",      eHTMLTag_applet,0},       {"AREA",      eHTMLTag_area,0},

  {"B",           eHTMLTag_bold,0},         {"BASE",      eHTMLTag_base,0},
  {"BASEFONT",    eHTMLTag_basefont,0},     {"BDO",       eHTMLTag_bdo,0},
  {"BIG",         eHTMLTag_big,0},          {"BLINK",     eHTMLTag_blink,0},
  {"BLOCKQUOTE",  eHTMLTag_blockquote,0},   {"BODY",      eHTMLTag_body,0},
  {"BR",          eHTMLTag_br,0},           {"BUTTON",    eHTMLTag_button,0},

  {"CAPTION",     eHTMLTag_caption,0},      {"CENTER",    eHTMLTag_center,0},
  {"CERTIFICATE", eHTMLTag_certificate,0},
  {"CITE",        eHTMLTag_cite,0},         {"CODE",      eHTMLTag_code,0},
  {"COL",         eHTMLTag_col,0},          {"COLGROUP",  eHTMLTag_colgroup,0},
  {"COMMENT",     eHTMLTag_comment,0},

  {"DD",          eHTMLTag_dd,0},           {"DEL",       eHTMLTag_del,0},
  {"DFN",         eHTMLTag_dfn,0},          {"DIR",       eHTMLTag_dir,0},
  {"DIV",         eHTMLTag_div,0},          {"DL",        eHTMLTag_dl,0},
  {"DT",          eHTMLTag_dt,0},

  {"EM",          eHTMLTag_em,0},           {"EMBED",     eHTMLTag_embed,0},
  {"ENTITY",      eHTMLTag_entity,0}, //a pseudo tag

  {"FIELDSET",    eHTMLTag_fieldset,0},     {"FONT",      eHTMLTag_font,0},
  {"FOOTER",      eHTMLTag_footer,0},       {"FORM",      eHTMLTag_form,0},
  {"FRAME",       eHTMLTag_frame,0},        {"FRAMESET",  eHTMLTag_frameset,0},     

  {"H1",          eHTMLTag_h1,0},           {"H2",        eHTMLTag_h2,0},
  {"H3",          eHTMLTag_h3,0},           {"H4",        eHTMLTag_h4,0},
  {"H5",          eHTMLTag_h5,0},           {"H6",        eHTMLTag_h6,0},
  {"HEAD",        eHTMLTag_head,0},         {"HEADER",    eHTMLTag_header,0},
  {"HR",          eHTMLTag_hr,0},           {"HTML",      eHTMLTag_html,0},
  
  {"I",           eHTMLTag_italic,0},       {"IFRAME",    eHTMLTag_iframe,0}, 
  {"ILAYER",      eHTMLTag_ilayer,0},       {"IMG",       eHTMLTag_img,0},          
  {"INPUT",       eHTMLTag_input,0},        {"INS",       eHTMLTag_ins,0},          
  {"ISINDEX",     eHTMLTag_isindex,0},      

  {"KBD",         eHTMLTag_kbd,0},          {"KEYGEN",    eHTMLTag_keygen,0},         

  {"LABEL",       eHTMLTag_label,0},        {"LAYER",     eHTMLTag_layer,0},        
  {"LEGEND",      eHTMLTag_legend,0},       {"LI",        eHTMLTag_listitem,0},     
  {"LINK",        eHTMLTag_link,0},         {"LISTING",   eHTMLTag_listing,0},      

  {"MAP",         eHTMLTag_map,0},          {"MARQUEE",   eHTMLTag_marquee,0},        
  {"MATH",        eHTMLTag_math,0},
  {"MENU",        eHTMLTag_menu,0},         {"META",      eHTMLTag_meta,0},         

  {"NEWLINE",     eHTMLTag_newline,0},      {"NOBR",      eHTMLTag_nobr,0},       

  {"NOEMBED",     eHTMLTag_noembed,0},      {"NOFRAMES",  eHTMLTag_noframes,0},     
  {"NOLAYER",     eHTMLTag_nolayer,0},      {"NOSCRIPT",  eHTMLTag_noscript,0},
  {"NOTE",        eHTMLTag_note,0},

  {"OBJECT",      eHTMLTag_object,0},       {"OL",        eHTMLTag_ol,0},           
  {"OPTION",      eHTMLTag_option,0},       

  {"P",           eHTMLTag_paragraph,0},    {"PARAM",     eHTMLTag_param,0},
  {"PLAINTEXT",   eHTMLTag_plaintext,0},

  {"PRE",         eHTMLTag_pre,0},

  {"Q",           eHTMLTag_quotation,0},

  {"S",           eHTMLTag_s,0},            {"SAMP",      eHTMLTag_samp,0},         
  {"SCRIPT",      eHTMLTag_script,0},       {"SELECT",    eHTMLTag_select,0},       
  {"SERVER",      eHTMLTag_server,0},       {"SMALL",     eHTMLTag_small,0},
  {"SPACER",      eHTMLTag_spacer,0},
  {"SPAN",        eHTMLTag_span,0},         {"SPELL",     eHTMLTag_spell,0},       
  {"STRIKE",      eHTMLTag_strike,0},       
  {"STRONG",      eHTMLTag_strong,0},       {"STYLE",     eHTMLTag_style,0},        
  {"SUB",         eHTMLTag_sub,0},          {"SUP",       eHTMLTag_sup,0},

  {"TABLE",       eHTMLTag_table,0},        {"TBODY",     eHTMLTag_tbody,0},    
  {"TD",          eHTMLTag_td,0},           
  
  {"TEXT",        eHTMLTag_text,0},

  {"TEXTAREA",    eHTMLTag_textarea,0},     
  {"TFOOT",       eHTMLTag_tfoot,0},        {"TH",        eHTMLTag_th,0},           
  {"THEAD",       eHTMLTag_thead,0},        {"TITLE",     eHTMLTag_title,0},        
  {"TR",          eHTMLTag_tr,0},           {"TT",        eHTMLTag_tt,0},

  {"U",           eHTMLTag_u,0},            {"UL",        eHTMLTag_ul,0},
  {"VAR",         eHTMLTag_var,0},          {"WBR",       eHTMLTag_wbr,0},
  {"WS",          eHTMLTag_whitespace,0},   
  
  {"X-USERDEF",   eHTMLTag_userdefined,0},  //make sure this is always last!

};

struct HTMLAttrEntry
{
  char            mName[11];
  eHTMLAttributes mAttrID;
};

HTMLAttrEntry gHTMLAttributeTable[] =
{
  {"ABBREV",     eHTMLAttr_abbrev},      {"ABOVE",     eHTMLAttr_above},
  {"ALT",         eHTMLAttr_alt},         {"ARRAY",     eHTMLAttr_array},
  {"AU",        eHTMLAttr_author},

  {"BACKGROUND",eHTMLAttr_background},  {"BANNER",    eHTMLAttr_banner},
  {"BELOW",     eHTMLAttr_below},       {"BGSOUND",   eHTMLAttr_bgsound},
  {"BOX",       eHTMLAttr_box},         {"BT",        eHTMLAttr_bt},

  {"CLASS",     eHTMLAttr_class},       {"COMMENT",   eHTMLAttr_comment}, 
  {"CREDIT",    eHTMLAttr_credit},

  {"DIR",       eHTMLAttr_dir},

  {"FIG",       eHTMLAttr_figure},      {"FIGURE",     eHTMLAttr_figure},
  {"FOOTNOTE",  eHTMLAttr_footnote},

  {"HEIGHT",    eHTMLAttr_height},

  {"ID",        eHTMLAttr_id},

  {"LANG",      eHTMLAttr_lang},

  {"MATH",       eHTMLAttr_math},

  {"NAME",      eHTMLAttr_name},        {"NEXTID",    eHTMLAttr_nextid},
  {"NOBR",      eHTMLAttr_nobreak},

  {"NOTE",      eHTMLAttr_note},

  {"OPTION",     eHTMLAttr_option},      {"OVERLAY",   eHTMLAttr_overlay},

  {"PERSON",     eHTMLAttr_person},      {"PUBLIC",    eHTMLAttr_public},

  {"RANGE",      eHTMLAttr_range},       {"ROOT",       eHTMLAttr_root},

  {"SGML",       eHTMLAttr_sgml},        {"SQRT",       eHTMLAttr_sqrt},
  {"SRC",        eHTMLAttr_src},         {"STYLE",      eHTMLAttr_style},

  {"TEXT",      eHTMLAttr_text},        {"TITLE",      eHTMLAttr_title},

  {"WBR",        eHTMLAttr_wordbreak},   {"WIDTH",     eHTMLAttr_width},

  {"XMP",       eHTMLAttr_xmp}
};


/*
 *  default constructor
 *  
 *  @update  gess 3/25/98
 *  @param   
 *  @return  
 */
CHTMLToken::CHTMLToken(const nsString& aName) : CToken(aName) {
  mTypeID=eHTMLTag_unknown;
}

/*
 *  constructor from tag id
 *  
 *  @update  gess 3/25/98
 *  @param   
 *  @return  
 */
CHTMLToken::CHTMLToken(eHTMLTags aTag) : CToken(GetTagName(aTag)) {
  mTypeID=aTag; 
}


/*
 *  default constructor
 *  
 *  @update  gess 3/25/98
 *  @param   
 *  @return  
 */
CStartToken::CStartToken(const nsString& aName) : CHTMLToken(aName) {
  mAttributed=PR_FALSE;
  mEmpty=PR_FALSE;
}

/*
 *  constructor from tag id
 *  
 *  @update  gess 3/25/98
 *  @param   
 *  @return  
 */
CStartToken::CStartToken(eHTMLTags aTag) : CHTMLToken(aTag) {
  mAttributed=PR_FALSE;
  mEmpty=PR_FALSE;
}

/*
 *  default destructor
 *  
 *  @update  gess 3/25/98
 *  @param   
 *  @return  
 */
PRInt32 CStartToken::GetTypeID(){
  if(eHTMLTag_unknown==mTypeID)
    mTypeID=DetermineHTMLTagType(mTextValue);
  return mTypeID;
}

/*
 *  
 *  
 *  @update  gess 3/25/98
 *  @param   
 *  @return  
 */
const char*  CStartToken::GetClassName(void) {
  return "start";
}

/*
 *  
 *  
 *  @update  gess 3/25/98
 *  @param   
 *  @return  
 */
PRInt32 CStartToken::GetTokenType(void) {
  return eToken_start;
}

/*
 *  
 *  
 *  @update  gess 3/25/98
 *  @param   
 *  @return  
 */
void CStartToken::SetAttributed(PRBool aValue) {
  mAttributed=aValue;
}

/*
 *  
 *  
 *  @update  gess 3/25/98
 *  @param   
 *  @return  
 */
PRBool CStartToken::IsAttributed(void) {
  return mAttributed;
}

/*
 *  
 *  
 *  @update  gess 3/25/98
 *  @param   
 *  @return  
 */
void CStartToken::SetEmpty(PRBool aValue) {
  mEmpty=aValue;
}

/*
 *  
 *  
 *  @update  gess 3/25/98
 *  @param   
 *  @return  
 */
PRBool CStartToken::IsEmpty(void) {
  return mEmpty;
}

/*
 *  Consume the identifier portion of the start tag
 *  
 *  @update  gess 3/25/98
 *  @param   aChar -- last char consumed from stream
 *  @param   aScanner -- controller of underlying input source
 *  @return  error result
 */
PRInt32 CStartToken::Consume(PRUnichar aChar, CScanner& aScanner) {

  //if you're here, we've already Consumed the < char, and are
   //ready to Consume the rest of the open tag identifier.
   //Stop consuming as soon as you see a space or a '>'.
   //NOTE: We don't Consume the tag attributes here, nor do we eat the ">"

  mTextValue=aChar;
  PRInt32 result=aScanner.ReadWhile(mTextValue,gIdentChars,PR_FALSE);

   //Good. Now, let's skip whitespace after the identifier,
   //and see if the next char is ">". If so, we have a complete
   //tag without attributes.
  if(kNoError==result) {  
    result=aScanner.SkipWhite();
    if(kNoError==result) {
      result=aScanner.GetChar(aChar);
      if(kNoError==result) {
        if(kGreaterThan!=aChar) { //look for '>' 
         //push that char back, since we apparently have attributes...
          aScanner.PutBack(aChar);
          mAttributed=PR_TRUE;
        } //if
      } //if
    }//if
  }
  return result;
};


/*
 *  Dump contents of this token to givne output stream
 *  
 *  @update  gess 3/25/98
 *  @param   out -- ostream to output content
 *  @return  
 */
void CStartToken::DebugDumpSource(ostream& out) {
  char* cp=mTextValue.ToNewCString();
  out << "<" << *cp;
  if(!mAttributed)
    out << ">";
  delete cp;
}


/*
 *  default constructor for end token
 *  
 *  @update  gess 3/25/98
 *  @param   aName -- char* containing token name
 *  @return  
 */
CEndToken::CEndToken(const nsString& aName) : CHTMLToken(aName) {
  mTypeID=eHTMLTag_unknown;
}

/*
 *  Consume the identifier portion of the end tag
 *  
 *  @update  gess 3/25/98
 *  @param   aChar -- last char consumed from stream
 *  @param   aScanner -- controller of underlying input source
 *  @return  error result
 */
PRInt32 CEndToken::Consume(PRUnichar aChar, CScanner& aScanner) {

  //if you're here, we've already Consumed the <! chars, and are
   //ready to Consume the rest of the open tag identifier.
   //Stop consuming as soon as you see a space or a '>'.
   //NOTE: We don't Consume the tag attributes here, nor do we eat the ">"

  mTextValue="";
  static nsAutoString terminals(">");
  PRInt32 result=aScanner.ReadUntil(mTextValue,terminals,PR_FALSE);
  if(kNoError==result)
    result=aScanner.GetChar(aChar); //eat the closing '>;
  return result;
};


/*
 *  Asks the token to determine the <i>HTMLTag type</i> of
 *  the token. This turns around and looks up the tag name
 *  in the tag dictionary.
 *  
 *  @update  gess 3/25/98
 *  @param   
 *  @return  eHTMLTag id of this endtag
 */
PRInt32 CEndToken::GetTypeID(){
  if(eHTMLTag_unknown==mTypeID)
    mTypeID=DetermineHTMLTagType(mTextValue);
  return mTypeID;
}

/*
 *  
 *  
 *  @update  gess 3/25/98
 *  @param   
 *  @return  
 */
const char*  CEndToken::GetClassName(void) {
  return "/end";
}

/*
 *  
 *  
 *  @update  gess 3/25/98
 *  @param   
 *  @return  
 */
PRInt32 CEndToken::GetTokenType(void) {
  return eToken_end;
}

/*
 *  Dump contents of this token to givne output stream
 *  
 *  @update  gess 3/25/98
 *  @param   out -- ostream to output content
 *  @return  
 */
void CEndToken::DebugDumpSource(ostream& out) {
  char* cp=mTextValue.ToNewCString();
  out << "</" << *cp << ">";
  delete cp;
}


/*
 *  Default constructor
 *  
 *  @update  gess 3/25/98
 *  @param   aName -- string to init token name with
 *  @return  
 */
CTextToken::CTextToken(const nsString& aName) : CHTMLToken(aName) {
  mTypeID=eHTMLTag_text;
}

/*
 *  
 *  
 *  @update  gess 3/25/98
 *  @param   
 *  @return  
 */
const char*  CTextToken::GetClassName(void) {
  return "text";
}

/*
 *  
 *  
 *  @update  gess 3/25/98
 *  @param   
 *  @return  
 */
PRInt32 CTextToken::GetTokenType(void) {
  return eToken_text;
}

/*
 *  Consume as much clear text from scanner as possible.
 *
 *  @update  gess 3/25/98
 *  @param   aChar -- last char consumed from stream
 *  @param   aScanner -- controller of underlying input source
 *  @return  error result
 */
PRInt32 CTextToken::Consume(PRUnichar aChar, CScanner& aScanner) {
  static nsAutoString terminals("&<\r\n");
  PRInt32 result=aScanner.ReadUntil(mTextValue,terminals,PR_FALSE);
  return result;
};


/*
 *  Default constructor
 *  
 *  @update  gess 3/25/98
 *  @param   
 *  @return  
 */
CCommentToken::CCommentToken(const nsString& aName) : CHTMLToken(aName) {
  mTypeID=eHTMLTag_comment;
}

/*
 *  Consume the identifier portion of the comment. 
 *  Note that we've already eaten the "<!" portion.
 *  
 *  @update  gess 3/25/98
 *  @param   aChar -- last char consumed from stream
 *  @param   aScanner -- controller of underlying input source
 *  @return  error result
 */
PRInt32 CCommentToken::Consume(PRUnichar aChar, CScanner& aScanner) {

  PRUnichar ch,ch2;
  PRInt32   result=kNoError;
  
  static nsAutoString terminals(">");
  
  aScanner.GetChar(ch);
  mTextValue="<!";
  if(kMinus==ch) {
    result=aScanner.GetChar(ch2);
    if(kNoError==result) {
      if(kMinus==ch2) {
           //in this case, we're reading a long-form comment <-- xxx -->
        mTextValue+="--";
        PRInt32 findpos=-1;
        while((findpos==kNotFound) && (kNoError==result)) {
          result=aScanner.ReadUntil(mTextValue,terminals,PR_TRUE);
          findpos=mTextValue.RFind("-->");
        }
      }
    }
    return result;
  }
     //if you're here, we're consuming a "short-form" comment
  mTextValue+=ch;
  result=aScanner.ReadUntil(mTextValue,terminals,PR_TRUE);
  return result;
};

/*
 *  
 *  
 *  @update  gess 3/25/98
 *  @param   
 *  @return  
 */
const char* CCommentToken::GetClassName(void){
  return "/**/";
}

/*
 *  
 *  
 *  @update  gess 3/25/98
 *  @param   
 *  @return  
 */
PRInt32 CCommentToken::GetTokenType(void) {
  return eToken_comment;
}

/*
 *  default constructor
 *  
 *  @update  gess 3/25/98
 *  @param   aName -- string value to init token name with
 *  @return  
 */
CNewlineToken::CNewlineToken(const nsString& aName) : CHTMLToken(aName) {
  mTypeID=eHTMLTag_newline;
}

/*
 *  
 *  
 *  @update  gess 3/25/98
 *  @param   
 *  @return  
 */
const char*  CNewlineToken::GetClassName(void) {
  return "crlf";
}

/*
 *  
 *  
 *  @update  gess 3/25/98
 *  @param   
 *  @return  
 */
PRInt32 CNewlineToken::GetTokenType(void) {
  return eToken_newline;
}

/**
 *  This method retrieves the value of this internal string. 
 *  
 *  @update gess 3/25/98
 *  @return nsString reference to internal string value
 */
nsString& CNewlineToken::GetText(void) {
  static nsAutoString theStr("\n");
  return theStr;
}

/*
 *  Consume as many cr/lf pairs as you can find.
 *  
 *  @update  gess 3/25/98
 *  @param   aChar -- last char consumed from stream
 *  @param   aScanner -- controller of underlying input source
 *  @return  error result
 */
PRInt32 CNewlineToken::Consume(PRUnichar aChar, CScanner& aScanner) {
  mTextValue=aChar;

    //we already read the \r or \n, let's see what's next!
  PRUnichar nextChar;
  PRInt32 result=aScanner.Peek(nextChar);

  if(kNoError==result) {
    switch(aChar) {
      case kNewLine:
        if(kCR==nextChar) {
          result=aScanner.GetChar(nextChar);
          mTextValue+=nextChar;
        }
        break;
      case kCR:
        if(kNewLine==nextChar) {
          result=aScanner.GetChar(nextChar);
          mTextValue+=nextChar;
        }
        break;
      default:
        break;
    }
  }  
  return result;
}

/*
 *  default constructor
 *  
 *  @update  gess 3/25/98
 *  @param   aName -- string value to init token name with
 *  @return  
 */
CAttributeToken::CAttributeToken(const nsString& aName) : CHTMLToken(aName),  
  mTextKey() {
  mLastAttribute=PR_FALSE;
}

/*
 *  
 *  
 *  @update  gess 3/25/98
 *  @param   
 *  @return  
 */
const char*  CAttributeToken::GetClassName(void) {
  return "attr";
}

/*
 *  
 *  
 *  @update  gess 3/25/98
 *  @param   
 *  @return  
 */
PRInt32 CAttributeToken::GetTokenType(void) {
  return eToken_attribute;
}

/*
 *  Dump contents of this token to givne output stream
 *  
 *  @update  gess 3/25/98
 *  @param   out -- ostream to output content
 *  @return  
 */
void CAttributeToken::DebugDumpToken(ostream& out) {
  char* cp=mTextKey.ToNewCString();
  out << "[" << GetClassName() << "] " << *cp << "=";
  delete cp;
  char* cp2=mTextValue.ToNewCString();
  out << *cp2 << ": " << mTypeID << endl;
}


/*
 *  This general purpose method is used when you want to
 *  consume a known quoted string. 
 *  
 *  @update  gess 3/25/98
 *  @param   aChar -- last char consumed from stream
 *  @param   aScanner -- controller of underlying input source
 *  @return  error result
 */
PRInt32 ConsumeQuotedString(PRUnichar aChar,nsString& aString,CScanner& aScanner){
  static nsAutoString terminals1(">'");
  static nsAutoString terminals2(">\"");

  PRInt32 result=kNotFound;
  switch(aChar) {
    case kQuote:
      result=aScanner.ReadUntil(aString,terminals2,PR_TRUE);
      break;
    case kApostrophe:
      result=aScanner.ReadUntil(aString,terminals1,PR_TRUE);
      break;
    default:
      break;
  }
  PRUnichar ch=aString.Last();
  if(ch!=aChar)
    aString+=aChar;
  return result;
}

/*
 *  This general purpose method is used when you want to
 *  consume attributed text value.
 *  
 *  @update  gess 3/25/98
 *  @param   aChar -- last char consumed from stream
 *  @param   aScanner -- controller of underlying input source
 *  @return  error result
 */
PRInt32 ConsumeAttributeValueText(PRUnichar aChar,nsString& aString,CScanner& aScanner){

  PRInt32 result=kNotFound;
  static nsAutoString terminals(" \t\b\r\n>");
  result=aScanner.ReadUntil(aString,terminals,PR_FALSE);
  return result;
}


/*
 *  Consume the key and value portions of the attribute.
 *  
 *  @update  gess 3/25/98
 *  @param   aChar -- last char consumed from stream
 *  @param   aScanner -- controller of underlying input source
 *  @return  error result
 */
PRInt32 CAttributeToken::Consume(PRUnichar aChar, CScanner& aScanner) {

  aScanner.SkipWhite();             //skip leading whitespace                                      
  PRInt32 result=aScanner.Peek(aChar);
  if(kNoError==result) {
    if(kQuote==aChar) {               //if you're here, handle quoted key...
      result=aScanner.GetChar(aChar);        //skip the quote sign...
      if(kNoError==result) {
        mTextKey=aChar;
        result=ConsumeQuotedString(aChar,mTextKey,aScanner);
      }
    }
    else if(kHashsign==aChar) {
      result=aScanner.GetChar(aChar);        //skip the hash sign...
      if(kNoError==result) {
        mTextKey=aChar;
        result=aScanner.ReadWhile(mTextKey,gDigits,PR_TRUE);
      }
    }
    else {
        //If you're here, handle an unquoted key.
        //Don't forget to reduce entities inline!
      static nsAutoString terminals(" >=\t\b\r\n\"");
      result=aScanner.ReadUntil(mTextKey,terminals,PR_FALSE);
    }

      //now it's time to Consume the (optional) value...
    if(!(result=aScanner.SkipWhite())) { 
      if(!(result=aScanner.Peek(aChar))) {
        if(kEqual==aChar){
          result=aScanner.GetChar(aChar);  //skip the equal sign...
          if(kNoError==result) {
            result=aScanner.SkipWhite();     //now skip any intervening whitespace
            if(kNoError==result) {
              result=aScanner.GetChar(aChar);  //and grab the next char.    
              if(kNoError==result) {
                if((kQuote==aChar) || (kApostrophe==aChar)) {
                  mTextValue=aChar;
                  result=ConsumeQuotedString(aChar,mTextValue,aScanner);
                }
                else {      
                  mTextValue=aChar;       //it's an alphanum attribute...
                  result=ConsumeAttributeValueText(aChar,mTextValue,aScanner);
                } 
              }//if
              if(kNoError==result)
                result=aScanner.SkipWhite();     
            }//if
          }//if
        }//if
      }//if
    }
    if(kNoError==result) {
      result=aScanner.Peek(aChar);
      mLastAttribute= PRBool((kGreaterThan==aChar) || (kEOF==result));
    }
  }
  return result;
}

/*
 *  Dump contents of this token to givne output stream
 *  
 *  @update  gess 3/25/98
 *  @param   out -- ostream to output content
 *  @return  
 */
void CAttributeToken::DebugDumpSource(ostream& out) {
  char* cp=mTextKey.ToNewCString();
  out << " " << *cp;
  delete cp;
  if(mTextValue.Length()) {
    cp=mTextValue.ToNewCString();
    out << "=" << *cp;
    delete cp;
  }
  if(mLastAttribute)
    out<<">";
}

/*
 *  default constructor
 *  
 *  @update  gess 3/25/98
 *  @param   aName -- string value to init token name with
 *  @return  
 */
CWhitespaceToken::CWhitespaceToken(const nsString& aName) : CHTMLToken(aName) {
  mTypeID=eHTMLTag_whitespace;
}

/*
 *  
 *  
 *  @update  gess 3/25/98
 *  @param   
 *  @return  
 */
const char*  CWhitespaceToken::GetClassName(void) {
  return "ws";
}

/*
 *  
 *  
 *  @update  gess 3/25/98
 *  @param   
 *  @return  
 */
PRInt32 CWhitespaceToken::GetTokenType(void) {
  return eToken_whitespace;
}

/*
 *  This general purpose method is used when you want to
 *  consume an aribrary sequence of whitespace. 
 *  
 *  @update  gess 3/25/98
 *  @param   aChar -- last char consumed from stream
 *  @param   aScanner -- controller of underlying input source
 *  @return  error result
 */
PRInt32 CWhitespaceToken::Consume(PRUnichar aChar, CScanner& aScanner) {
  
  mTextValue=aChar;

  PRInt32 result=aScanner.ReadWhile(mTextValue,gWhitespace,PR_FALSE);
  if(kNoError==result) {
    mTextValue.StripChars("\r");
  }
  return result;
}

/*
 *  default constructor
 *  
 *  @update  gess 3/25/98
 *  @param   aName -- string value to init token name with
 *  @return  
 */
CEntityToken::CEntityToken(const nsString& aName) : CHTMLToken(aName) {
  mTypeID=eHTMLTag_entity;
#ifdef VERBOSE_DEBUG
  if(!VerifyEntityTable())  {
    cout<<"Entity table is invalid!" << endl;
  }
#endif
}

/*
 *  Consume the rest of the entity. We've already eaten the "&".
 *  
 *  @update  gess 3/25/98
 *  @param   aChar -- last char consumed from stream
 *  @param   aScanner -- controller of underlying input source
 *  @return  error result
 */
PRInt32 CEntityToken::Consume(PRUnichar aChar, CScanner& aScanner) {
  if(aChar)
    mTextValue=aChar;
  PRInt32 result=ConsumeEntity(aChar,mTextValue,aScanner);
  return result;
}

/*
 *  
 *  
 *  @update  gess 3/25/98
 *  @param   
 *  @return  
 */
const char*  CEntityToken::GetClassName(void) {
  return "&entity";
}

/*
 *  
 *  
 *  @update  gess 3/25/98
 *  @param   
 *  @return  
 */
PRInt32 CEntityToken::GetTokenType(void) {
  return eToken_entity;
}

/*
 *  This general purpose method is used when you want to
 *  consume an entity &xxxx;. Keep in mind that entities
 *  are <i>not</i> reduced inline.
 *  
 *  @update  gess 3/25/98
 *  @param   aChar -- last char consumed from stream
 *  @param   aScanner -- controller of underlying input source
 *  @return  error result
 */
PRInt32 CEntityToken::ConsumeEntity(PRUnichar aChar,nsString& aString,CScanner& aScanner){

  PRInt32 result=aScanner.Peek(aChar);
  if(kNoError==result) {
    if(kLeftBrace==aChar) {
      //you're consuming a script entity...
      static nsAutoString terminals("}>");
      result=aScanner.ReadUntil(aString,terminals,PR_FALSE);
      if(kNoError==result) {
        result=aScanner.Peek(aChar);
        if(kNoError==result) {
          if(kRightBrace==aChar) {
            aString+=kRightBrace;   //append rightbrace, and...
            result=aScanner.GetChar(aChar);//yank the closing right-brace
          }
        }
      }
    } //if
    else {
      result=aScanner.ReadWhile(aString,gIdentChars,PR_FALSE);
      if(kNoError==result) {
        result=aScanner.Peek(aChar);
        if(kNoError==result) {
          if (kSemicolon == aChar) {
            // consume semicolon that stopped the scan
            result=aScanner.GetChar(aChar);
          }
        }
      }//if
    } //else
  } //if
  return result;
}


/*
 *  This method converts this entity into its underlying
 *  unicode equivalent.
 *  
 *  @update  gess 3/25/98
 *  @param   
 *  @return  
 */
PRInt32 CEntityToken::TranslateToUnicodeStr(nsString& aString) {
  PRInt32 index=0;
  if(aString.IsDigit(mTextValue[0])) {
    PRInt32 err=0;
    index=mTextValue.ToInteger(&err);
    if(0==err)
      aString.Append(PRUnichar(index));
  }
  else {
    index=FindEntityIndex(mTextValue);
    if(kNotFound!=index) {
      PRUnichar ch=gStrToUnicodeTable[index].mValue;
      aString=ch;
    } 
    else {
#ifdef GESS_MACHINE
      index=TranslateExtendedEntity(mTextValue,aString);
#endif
    }
  }
  return index;
}

 

/*
 *  This method ensures that the entity table doesn't get
 *  out of sync. Make sure you call this at least once.
 *  
 *  @update  gess 3/25/98
 *  @return  PR_TRUE if valid (ordered correctly)
 */
PRBool CEntityToken::VerifyEntityTable(){
  PRInt32  count=sizeof(gStrToUnicodeTable)/sizeof(StrToUnicodeStruct);
  PRInt32 i,j;
  for(i=1;i<count-1;i++)
  {
    j=strcmp(gStrToUnicodeTable[i-1].mName,gStrToUnicodeTable[i].mName);
    if(j>0)
      return PR_FALSE;
  }
  return PR_TRUE;
}


/*
 *  This method is used to convert from a given string (char*)
 *  into a entity index (offset within entity table).
 *  
 *  @update  gess 3/25/98
 *  @param   aBuffer -- string to be converted
 *  @param   aBuflen -- optional string length
 *  @return  integer offset of string in table, or kNotFound
 */
PRInt32 CEntityToken::FindEntityIndex(nsString& aString) {
  PRInt32  result=kNotFound;
  PRInt32  cnt=sizeof(gStrToUnicodeTable)/sizeof(StrToUnicodeStruct);
  PRInt32  low=0; 
  PRInt32  high=cnt-1;
  PRInt32  middle=kNotFound;
    
  if(cnt) {
    while(low<=high)
    {
      middle=(PRInt32)(low+high)/2;
      result=aString.Compare(gStrToUnicodeTable[middle].mName);
//      result=strcmp(aBuffer,gStrToUnicodeTable[middle].mName);
      if (result==0) {
        return middle;
      }
      if (result<0) {
        high=middle-1;
      }
      else low=middle+1; 
    }
  }
  return kNotFound;
}


/*
 *  This method is used to convert from a given string (char*)
 *  into a entity index (offset within entity table).
 *  
 *  @update  gess 3/25/98
 *  @param   aBuffer -- string to be converted
 *  @param   aBuflen -- optional string length
 *  @return  integer offset of string in table, or kNotFound
 */
PRInt32 CEntityToken::FindEntityIndexMax(const char* aBuffer,PRInt32 aBufLen) {
  PRInt32  result=kNotFound;
  PRInt32  cnt=sizeof(gStrToUnicodeTable)/sizeof(StrToUnicodeStruct);
  PRInt32  low=0; 
  PRInt32  high=cnt-1;
  PRInt32  middle=kNotFound;
  
  if(aBuffer) {
    if(-1==aBufLen) {
      aBufLen=strlen(aBuffer);
    }
  
    if(aBufLen && cnt) {
      while(low<=high)
      {
        middle=(PRInt32)(low+high)/2;
        result=strcmp(aBuffer,gStrToUnicodeTable[middle].mName);
        if (result==0) {
          return middle;
        }
        if (result<0) {
          high=middle-1;
        }
        else low=middle+1; 
      }
    }
  }
  return kNotFound;
}


/*
 *  This method reduces all text entities into their char
 *  representation.
 *  
 *  @update  gess 3/25/98
 *  @param   
 *  @return  
 */
PRInt32 CEntityToken::ReduceEntities(nsString& aString) {
  PRInt32 result=0;
  PRInt32 amppos=0;
  PRBool done=PR_FALSE;
  PRInt32 offset=0;

  while(!done) {
    if(kNotFound!=(amppos=aString.Find('&',offset))) {
      if(!nsString::IsSpace(aString[amppos+1])) { //have we found a genuine entity?
        PRInt32 endpos=aString.FindLastCharInSet(gIdentChars,amppos+1);
        PRInt32 cnt;
        if(kNotFound==endpos) 
          cnt=aString.Length()-1-amppos;
        else cnt=endpos-amppos;
        PRInt32 index=FindEntityIndexMax((const char*)&aString[amppos+1],cnt);
        if(kNotFound!=index) {
          aString[amppos]=gStrToUnicodeTable[index].mValue;
          aString.Cut(amppos+1,cnt+(endpos!=kNotFound));
        }
        else offset=amppos+1;
      }
    }
    else done=PR_TRUE;
  }
  return result;
}

/*
 *  Dump contents of this token to givne output stream
 *  
 *  @update  gess 3/25/98
 *  @param   out -- ostream to output content
 *  @return  
 */
void CEntityToken::DebugDumpSource(ostream& out) {
  char* cp=mTextValue.ToNewCString();
  out << "&" << *cp;
  delete cp;
}

/*
 *  
 *  
 *  @update  gess 3/25/98
 *  @param   
 *  @return  
 */
const char*  CScriptToken::GetClassName(void) {
  return "script";
}

/*
 *  
 *  
 *  @update  gess 3/25/98
 *  @param   
 *  @return  
 */
PRInt32 CScriptToken::GetTokenType(void) {
  return eToken_script;
}

/*
 *  
 *  
 *  @update  gess 3/25/98
 *  @param   
 *  @return  
 */
const char*  CStyleToken::GetClassName(void) {
  return "style";
}

/*
 *  
 *  
 *  @update  gess 3/25/98
 *  @param   
 *  @return  
 */
PRInt32 CStyleToken::GetTokenType(void) {
  return eToken_style;
}


/*
 *  default constructor
 *  
 *  @update  gess 3/25/98
 *  @param   aName -- string value to init token name with
 *  @return  
 */
CSkippedContentToken::CSkippedContentToken(const nsString& aName) : CAttributeToken(aName) {
  mTextKey = "$skipped-content";/* XXX need a better answer! */
}

/*
 *  
 *  
 *  @update  gess 3/25/98
 *  @param   
 *  @return  
 */
const char*  CSkippedContentToken::GetClassName(void) {
  return "skipped";
}

/*
 *  
 *  
 *  @update  gess 3/25/98
 *  @param   
 *  @return  
 */
PRInt32 CSkippedContentToken::GetTokenType(void) {
  return eToken_skippedcontent;
}

/*
 *  Consume content until you find a sequence that matches
 *  this objects mTextValue.
 *  
 *  @update  gess 3/25/98
 *  @param   aChar -- last char consumed from stream
 *  @param   aScanner -- controller of underlying input source
 *  @return  error result
 */
PRInt32 CSkippedContentToken::Consume(PRUnichar aChar,CScanner& aScanner) {
  PRBool      done=PR_FALSE;
  PRInt32     result=kNoError;
  nsString    temp;

//  while((!done) && (!aScanner.Eof())) {
  while((!done) && (kNoError==result)) {
    static nsAutoString terminals(">");
    result=aScanner.ReadUntil(temp,terminals,PR_TRUE);
    done=PRBool(kNotFound!=temp.RFind(mTextValue,PR_TRUE));
  }
  mTextValue=temp;
  return result;
}


/*
 *  This method iterates the tagtable to ensure that is 
 *  is proper sort order. This method only needs to be
 *  called once.
 *  
 *  @update  gess 3/25/98
 *  @param   
 *  @return  
 */
class CTagTableVerifier {
public:
  CTagTableVerifier(){
    PRInt32  count=sizeof(gHTMLTagTable)/sizeof(HTMLTagEntry);
    PRInt32 i,j;
    for(i=1;i<count-1;i++)
    {
      j=strcmp(gHTMLTagTable[i-1].mName,gHTMLTagTable[i].mName);
      if(j>0) {
        cout << "Tag Table names are out of order at " << i << "!" << endl;
      }
      if(gHTMLTagTable[i-1].mTagID>=gHTMLTagTable[i].mTagID) {
        cout << "Tag table ID's are out of order at " << i << "!" << endl;;
      }
    }
  }
};



/*
 *  This method accepts a string (and optionally, its length)
 *  and determines the eHTMLTag (id) value.
 *  
 *  @update  gess 3/25/98
 *  @param   aString -- string to be convered to id
 *  @return  valid id, or user_defined.
 */
eHTMLTags DetermineHTMLTagType(const nsString& aString)
{
  PRInt32  result=-1;
  PRInt32  cnt=sizeof(gHTMLTagTable)/sizeof(HTMLTagEntry);
  PRInt32  low=0; 
  PRInt32  high=cnt-1;
  PRInt32  middle=kNotFound;
  
  while(low<=high){
    middle=(PRInt32)(low+high)/2;
    result=aString.Compare(gHTMLTagTable[middle].mName, PR_TRUE);
    if (result==0)
      return gHTMLTagTable[middle].mTagID; 
    if (result<0)
      high=middle-1; 
    else low=middle+1; 
  }
  return eHTMLTag_userdefined;
}


/**
 * 
 * @update	gess4/25/98
 * @param 
 * @return
 */
const char* GetTagName(PRInt32 aTag) {
  const    char* result=0;
  PRInt32  cnt=sizeof(gHTMLTagTable)/sizeof(HTMLTagEntry);
  PRInt32  low=0; 
  PRInt32  high=cnt-1;
  PRInt32  middle=kNotFound;
  
  while(low<=high) {
    middle=(PRInt32)(low+high)/2;
    if(aTag==gHTMLTagTable[middle].mTagID)
      return gHTMLTagTable[middle].mName;
    if(aTag<gHTMLTagTable[middle].mTagID)
      high=middle-1; 
    else low=middle+1; 
  }
  return gUserdefined;
}


/*
 *  This method iterates the attribute-table to ensure that is 
 *  is proper sort order. This method only needs to be
 *  called once.
 *  
 *  @update  gess 3/25/98
 *  @param   
 *  @return  
 */
class CAttributeTableVerifier {
public:
  CAttributeTableVerifier(){
    PRInt32  count=sizeof(gHTMLAttributeTable)/sizeof(HTMLAttrEntry);
    PRInt32 i,j;
    for(i=1;i<count-1;i++)
    {
      j=strcmp(gHTMLAttributeTable[i-1].mName,gHTMLAttributeTable[i].mName);
      if(j>0) {
#ifdef VERBOSE_DEBUG
        cout << "Attribute table is out of order at " << j << "!" << endl;
#endif
        return;
      }
    }
    return;
  }
};


/*
 *  These objects are here to force the validation of the
 *  tag and attribute tables.
 */

CAttributeTableVerifier gAttributeTableVerifier;
CTagTableVerifier gTableVerifier;



