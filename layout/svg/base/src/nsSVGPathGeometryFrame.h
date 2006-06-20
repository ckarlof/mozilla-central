/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* ***** BEGIN LICENSE BLOCK *****
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
 * The Original Code is the Mozilla SVG project.
 *
 * The Initial Developer of the Original Code is
 * Crocodile Clips Ltd..
 * Portions created by the Initial Developer are Copyright (C) 2002
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Alex Fritze <alex.fritze@crocodile-clips.com> (original author)
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either of the GNU General Public License Version 2 or later (the "GPL"),
 * or the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
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

#ifndef __NS_SVGPATHGEOMETRYFRAME_H__
#define __NS_SVGPATHGEOMETRYFRAME_H__

#include "nsFrame.h"
#include "nsISVGChildFrame.h"
#include "nsWeakReference.h"
#include "nsISVGValue.h"
#include "nsISVGValueObserver.h"
#include "nsLayoutAtoms.h"
#include "nsSVGGeometryFrame.h"

class nsPresContext;
class nsIDOMSVGMatrix;
class nsSVGMarkerFrame;
class nsISVGFilterFrame;
struct nsSVGMarkerProperty;
class nsISVGCairoCanvas;

typedef nsSVGGeometryFrame nsSVGPathGeometryFrameBase;

#define HITTEST_MASK_FILL 1
#define HITTEST_MASK_STROKE 2

class nsSVGPathGeometryFrame : public nsSVGPathGeometryFrameBase,
                               public nsISVGChildFrame
{
protected:
  nsSVGPathGeometryFrame(nsStyleContext* aContext);
  virtual ~nsSVGPathGeometryFrame();
  
public:
   // nsISupports interface:
  NS_IMETHOD QueryInterface(const nsIID& aIID, void** aInstancePtr);
  NS_IMETHOD_(nsrefcnt) AddRef() { return 1; }
  NS_IMETHOD_(nsrefcnt) Release() { return 1; }

  // nsIFrame interface:
  NS_IMETHOD
  Init(nsIContent*      aContent,
       nsIFrame*        aParent,
       nsIFrame*        aPrevInFlow);

  NS_IMETHOD  AttributeChanged(PRInt32         aNameSpaceID,
                               nsIAtom*        aAttribute,
                               PRInt32         aModType);

  NS_IMETHOD DidSetStyleContext();

  /**
   * Get the "type" of the frame
   *
   * @see nsLayoutAtoms::svgPathGeometryFrame
   */
  virtual nsIAtom* GetType() const;
  virtual PRBool IsFrameOfType(PRUint32 aFlags) const;

#ifdef DEBUG
  NS_IMETHOD GetFrameName(nsAString& aResult) const
  {
    return MakeFrameName(NS_LITERAL_STRING("SVGPathGeometry"), aResult);
  }
#endif

  // nsISVGGeometrySource interface:
  NS_IMETHOD GetCanvasTM(nsIDOMSVGMatrix * *aCTM);

protected:
  // nsISVGValueObserver
  NS_IMETHOD WillModifySVGObservable(nsISVGValue* observable,
                                     nsISVGValue::modificationType aModType);
  NS_IMETHOD DidModifySVGObservable (nsISVGValue* observable,
                                     nsISVGValue::modificationType aModType);

  // nsISVGChildFrame interface:
  NS_IMETHOD PaintSVG(nsISVGRendererCanvas* canvas);
  NS_IMETHOD GetFrameForPointSVG(float x, float y, nsIFrame** hit);
  NS_IMETHOD_(nsRect) GetCoveredRegion();
  NS_IMETHOD UpdateCoveredRegion();
  NS_IMETHOD InitialUpdate();
  NS_IMETHOD NotifyCanvasTMChanged(PRBool suppressInvalidation);
  NS_IMETHOD NotifyRedrawSuspended();
  NS_IMETHOD NotifyRedrawUnsuspended();
  NS_IMETHOD SetMatrixPropagation(PRBool aPropagate);
  NS_IMETHOD SetOverrideCTM(nsIDOMSVGMatrix *aCTM);
  NS_IMETHOD GetBBox(nsIDOMSVGRect **_retval);
  
  // nsISVGGeometrySource interface:
  virtual nsresult UpdateGraphic(PRBool suppressInvalidation = PR_FALSE);
  
protected:
  virtual PRBool IsMarkable() { return PR_FALSE; }
  virtual void GetMarkPoints(nsVoidArray *aMarks) {}
  virtual void ConstructPath(cairo_t *aCtx) = 0;
  virtual PRUint16 GetHittestMask();

  void Render(nsISVGRendererCanvas *aCanvas);
  void GeneratePath(cairo_t *ctx, nsISVGCairoCanvas* aCanvas);

private:
  nsSVGMarkerProperty *GetMarkerProperty();
  void GetMarkerFromStyle(nsSVGMarkerFrame   **aResult,
                          nsSVGMarkerProperty *property,
                          nsIURI              *aURI);
  void UpdateMarkerProperty();

  nsCOMPtr<nsIDOMSVGMatrix> mOverrideCTM;
  PRPackedBool mPropagateTransform;
};

#endif // __NS_SVGPATHGEOMETRYFRAME_H__
