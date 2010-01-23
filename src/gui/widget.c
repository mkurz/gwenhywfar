/***************************************************************************
    begin       : Wed Jan 20 2010
    copyright   : (C) 2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Lesser General Public            *
 *   License as published by the Free Software Foundation; either          *
 *   version 2.1 of the License, or (at your option) any later version.    *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston,                 *
 *   MA  02111-1307  USA                                                   *
 *                                                                         *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "widget_p.h"

#include <gwenhywfar/text.h>
#include <gwenhywfar/debug.h>

#include <assert.h>
#include <ctype.h>



GWEN_INHERIT_FUNCTIONS(GWEN_WIDGET)
GWEN_TREE_FUNCTIONS(GWEN_WIDGET, GWEN_Widget)




GWEN_WIDGET *GWEN_Widget_new(GWEN_DIALOG *dlg) {
  GWEN_WIDGET *w;

  GWEN_NEW_OBJECT(GWEN_WIDGET, w);
  w->refCount=1;
  GWEN_INHERIT_INIT(GWEN_WIDGET, w);
  GWEN_TREE_INIT(GWEN_WIDGET, w);

  w->dialog=dlg;

  return w;
}



void GWEN_Widget_free(GWEN_WIDGET *w) {
  if (w) {
    assert(w->refCount);
    if (w->refCount>1) {
      w->refCount--;
    }
    else {
      GWEN_TREE_FINI(GWEN_WIDGET, w);
      GWEN_INHERIT_FINI(GWEN_WIDGET, w);
      free(w->name);
      free(w->text);
      w->refCount=0;
      GWEN_FREE_OBJECT(w);
    }
  }
}



GWEN_DIALOG *GWEN_Widget_GetDialog(const GWEN_WIDGET *w) {
  assert(w);
  assert(w->refCount);
  return w->dialog;
}



void *GWEN_Widget_GetImplData(const GWEN_WIDGET *w) {
  assert(w);
  assert(w->refCount);
  return w->impl_data;
}



void GWEN_Widget_SetImplData(GWEN_WIDGET *w, void *ptr) {
  assert(w);
  assert(w->refCount);
  w->impl_data=ptr;
}



uint32_t GWEN_Widget_GetFlags(const GWEN_WIDGET *w) {
  assert(w);
  assert(w->refCount);
  return w->flags;
}



void GWEN_Widget_SetFlags(GWEN_WIDGET *w, uint32_t fl) {
  assert(w);
  assert(w->refCount);
  w->flags=fl;
}



void GWEN_Widget_AddFlags(GWEN_WIDGET *w, uint32_t fl) {
  assert(w);
  assert(w->refCount);
  w->flags|=fl;
}



void GWEN_Widget_SubFlags(GWEN_WIDGET *w, uint32_t fl) {
  assert(w);
  assert(w->refCount);
  w->flags&=~fl;
}



GWEN_WIDGET_TYPE GWEN_Widget_GetType(const GWEN_WIDGET *w) {
  assert(w);
  assert(w->refCount);
  return w->wtype;
}



void GWEN_Widget_SetType(GWEN_WIDGET *w, GWEN_WIDGET_TYPE t) {
  assert(w);
  assert(w->refCount);
  w->wtype=t;
}



int GWEN_Widget_GetColumns(const GWEN_WIDGET *w) {
  assert(w);
  assert(w->refCount);
  return w->columns;
}



void GWEN_Widget_SetColumns(GWEN_WIDGET *w, int i) {
  assert(w);
  assert(w->refCount);
  w->columns=i;
}



int GWEN_Widget_GetRows(const GWEN_WIDGET *w) {
  assert(w);
  assert(w->refCount);
  return w->rows;
}



void GWEN_Widget_SetRows(GWEN_WIDGET *w, int i) {
  assert(w);
  assert(w->refCount);
  w->rows=i;
}



const char *GWEN_Widget_GetText(const GWEN_WIDGET *w) {
  assert(w);
  assert(w->refCount);
  return w->text;
}



void GWEN_Widget_SetText(GWEN_WIDGET *w, const char *s) {
  assert(w);
  assert(w->refCount);
  free(w->text);
  if (s) w->text=strdup(s);
  else w->text=NULL;
}



const char *GWEN_Widget_GetName(const GWEN_WIDGET *w) {
  assert(w);
  assert(w->refCount);
  return w->name;
}



void GWEN_Widget_SetName(GWEN_WIDGET *w, const char *s) {
  assert(w);
  assert(w->refCount);
  free(w->name);
  if (s) w->name=strdup(s);
  else w->name=NULL;
}



GWEN_WIDGET_TYPE GWEN_Widget_Type_fromString(const char *s) {
  if (s && *s) {
    if (strcasecmp(s, "unknown")==0)
      return GWEN_Widget_TypeUnknown;
    else if (strcasecmp(s, "none")==0)
      return GWEN_Widget_TypeNone;
    else if (strcasecmp(s, "label")==0)
      return GWEN_Widget_TypeLabel;
    else if (strcasecmp(s, "pushButton")==0)
      return GWEN_Widget_TypePushButton;
    else if (strcasecmp(s, "lineEdit")==0)
      return GWEN_Widget_TypeLineEdit;
    else if (strcasecmp(s, "textEdit")==0)
      return GWEN_Widget_TypeTextEdit;
    else if (strcasecmp(s, "comboBox")==0)
      return GWEN_Widget_TypeComboBox;
    else if (strcasecmp(s, "radioButton")==0)
      return GWEN_Widget_TypeRadioButton;
    else if (strcasecmp(s, "progressBar")==0)
      return GWEN_Widget_TypeProgressBar;
    else if (strcasecmp(s, "radioGroup")==0)
      return GWEN_Widget_TypeRadioGroup;
    else if (strcasecmp(s, "groupBox")==0)
      return GWEN_Widget_TypeGroupBox;
    else if (strcasecmp(s, "hSpacer")==0)
      return GWEN_Widget_TypeHSpacer;
    else if (strcasecmp(s, "vSpacer")==0)
      return GWEN_Widget_TypeVSpacer;
    else if (strcasecmp(s, "hLayout")==0)
      return GWEN_Widget_TypeHLayout;
    else if (strcasecmp(s, "vLayout")==0)
      return GWEN_Widget_TypeVLayout;
    else if (strcasecmp(s, "gridLayout")==0)
      return GWEN_Widget_TypeGridLayout;
    else if (strcasecmp(s, "image")==0)
      return GWEN_Widget_TypeImage;
    else if (strcasecmp(s, "listBox")==0)
      return GWEN_Widget_TypeListBox;
    else if (strcasecmp(s, "dialog")==0)
      return GWEN_Widget_TypeDialog;
    else if (strcasecmp(s, "tabBook")==0)
      return GWEN_Widget_TypeTabBook;
    else if (strcasecmp(s, "tabPage")==0)
      return GWEN_Widget_TypeTabPage;
    else if (strcasecmp(s, "wizard")==0)
      return GWEN_Widget_TypeWizard;
    else if (strcasecmp(s, "wizardPage")==0)
      return GWEN_Widget_TypeWizardPage;
  }
  return GWEN_Widget_TypeUnknown;
}



const char *GWEN_Widget_Type_toString(GWEN_WIDGET_TYPE t) {
  switch(t) {
  case GWEN_Widget_TypeNone:            return "none";
  case GWEN_Widget_TypeLabel:           return "label";
  case GWEN_Widget_TypePushButton:      return "pushButton";
  case GWEN_Widget_TypeLineEdit:        return "lineEdit";
  case GWEN_Widget_TypeTextEdit:        return "textEdit";
  case GWEN_Widget_TypeComboBox:        return "comboBox";
  case GWEN_Widget_TypeRadioButton:     return "radioButton";
  case GWEN_Widget_TypeProgressBar:     return "progressBar";
  case GWEN_Widget_TypeRadioGroup:      return "radioGroup";
  case GWEN_Widget_TypeGroupBox:        return "groupBox";
  case GWEN_Widget_TypeHSpacer:         return "hSpacer";
  case GWEN_Widget_TypeVSpacer:         return "vSpacer";
  case GWEN_Widget_TypeHLayout:         return "hLayout";
  case GWEN_Widget_TypeVLayout:         return "vLayout";
  case GWEN_Widget_TypeGridLayout:      return "gridLayout";
  case GWEN_Widget_TypeImage:           return "image";
  case GWEN_Widget_TypeListBox:         return "listBox";
  case GWEN_Widget_TypeDialog:          return "dialog";
  case GWEN_Widget_TypeTabBook:         return "tabBook";
  case GWEN_Widget_TypeTabPage:         return "tabPage";
  case GWEN_Widget_TypeWizard:          return "wizard";
  case GWEN_Widget_TypeWizardPage:      return "wizardPage";
  case GWEN_Widget_TypeUnknown:
  default:                              return "unknown";
  }
}



uint32_t GWEN_Widget_Flags_fromString(const char *s){
  uint32_t fl=0;

  if (s && *s) {
    char *copy;
    char *p;

    copy=strdup(s);
    p=copy;

    while(*p) {
      char *wstart;

      /* skip blanks */
      while(*p && isspace(*p))
	p++;
      /* save start of word */
      wstart=p;

      /* find end of word */
      while(*p && !(isspace(*p) || *p==','))
	p++;
      if (*p)
	/* set blank or comma to 0, advance pointer */
	*(p++)=0;

      /* parse flags */
      if (strcasecmp(wstart, "fillX")==0)
        fl|=GWEN_WIDGET_FLAGS_FILLX;
      else if (strcasecmp(wstart, "fillY")==0)
	fl|=GWEN_WIDGET_FLAGS_FILLY;
    }
  }

  return fl;
}



int GWEN_Widget_ReadXml(GWEN_WIDGET *w, GWEN_XMLNODE *node) {
  const char *s;

  s=GWEN_XMLNode_GetProperty(node, "type", "unknown");
  if (s && *s)
    w->wtype=GWEN_Widget_Type_fromString(s);
  else {
    DBG_ERROR(GWEN_LOGDOMAIN, "type property missing in node");
    return GWEN_ERROR_BAD_DATA;
  }

  s=GWEN_XMLNode_GetProperty(node, "flags", NULL);
  if (s && *s)
    w->flags=GWEN_Widget_Flags_fromString(s);

  w->columns=GWEN_XMLNode_GetIntValue(node, "columns", 0);
  w->rows=GWEN_XMLNode_GetIntValue(node, "rows", 0);

  s=GWEN_XMLNode_GetProperty(node, "text", NULL);
  if (s && *s)
    GWEN_Widget_SetText(w, s);

  return 0;
}






