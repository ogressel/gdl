/***************************************************************************
                       plotting.cpp  -  GDL routines for plotting
                             -------------------
    begin                : July 22 2002
    copyright            : (C) 2002-2011 by Marc Schellens et al.
    email                : m_schellens@users.sf.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "includefirst.hpp"
#include "plotting.hpp"

namespace lib {

  using namespace std;

  class axis_call : public plotting_routine_call 
  {

    private: bool handle_args(EnvT* e)
    {
      return true;
    } 

  private: void old_body( EnvT* e, GDLGStream* actStream) 
  { 
    bool doT3d, real3d;
    DDouble zValue;
    //note: Z (VALUE) will be used uniquely if Z is not effectively defined.
    static int zvIx = e->KeywordIx( "Z");
    zValue=0.0;
    e->AssureDoubleScalarKWIfPresent ( zvIx, zValue );
    //T3D
    static int t3dIx = e->KeywordIx( "T3D");
    doT3d=(e->KeywordSet(t3dIx) || T3Denabled());

    DFloat xMarginL, xMarginR, yMarginB, yMarginT; 

    bool xAxis=false, yAxis=false, zAxis=false;
    static int xaxisIx = e->KeywordIx( "XAXIS");
    static int yaxisIx = e->KeywordIx( "YAXIS");
    static int zaxisIx = e->KeywordIx( "ZAXIS");
    
    PLINT xaxis_value, yaxis_value, zaxis_value;
    bool standardNumPos;
    //IDL behaviour for XAXIS and YAXIS and ZAXIS options: only one option is considered, and ZAXIS above YAXIS above XAXIS
    if( (e->GetKW( xaxisIx) != NULL) ) {
      xAxis = true;
      e->AssureLongScalarKWIfPresent(xaxisIx, xaxis_value);
      if (xaxis_value == 0) {standardNumPos = true;} else {standardNumPos = false;}
    }      
    if( e->GetKW( yaxisIx) != NULL) {
      yAxis = true; xAxis = false; // like in IDL, yaxis overrides xaxis
      e->AssureLongScalarKWIfPresent( yaxisIx, yaxis_value);
      if (yaxis_value == 0) {standardNumPos = true;} else {standardNumPos = false;}
    }
    if( e->GetKW( zaxisIx) != NULL) {
      zAxis = true; xAxis = false; yAxis=false; // like in IDL, zaxis overrides all
      e->AssureLongScalarKWIfPresent( zaxisIx, zaxis_value);
    }
    if( (e->GetKW( xaxisIx) == NULL) && (e->GetKW( yaxisIx) == NULL )  && ((e->GetKW( zaxisIx) == NULL )||!doT3d))  {
      xAxis = true; standardNumPos = true; 
    }
    
    // MARGIN
    gdlGetDesiredAxisMargin(e, XAXIS,xMarginL, xMarginR);
    gdlGetDesiredAxisMargin(e, YAXIS,yMarginB, yMarginT);

    // will handle axis logness..
    bool xLog, yLog, zLog;
    // is current box log or not?
    bool xAxisWasLog, yAxisWasLog, zAxisWasLog;
    gdlGetAxisType(XAXIS, xAxisWasLog);
    gdlGetAxisType(YAXIS, yAxisWasLog);
    gdlGetAxisType(ZAXIS, zAxisWasLog);
    xLog=xAxisWasLog;
    yLog=yAxisWasLog; //by default logness is similar until another option is set
    zLog=zAxisWasLog;
    
    enum
    {
      DATA=0,
      NORMAL,
      DEVICE
    } coordinateSystem=DATA;
    static int DATAIx=e->KeywordIx("DATA");
    static int DEVICEIx=e->KeywordIx("DEVICE");
    static int NORMALIx=e->KeywordIx("NORMAL");
    static int SAVEIx=e->KeywordIx("SAVE");
    //check presence of DATA,DEVICE and NORMAL options
    if ( e->KeywordSet(DATAIx) ) coordinateSystem=DATA;
    if ( e->KeywordSet(DEVICEIx) ) coordinateSystem=DEVICE;
    if ( e->KeywordSet(NORMALIx) ) coordinateSystem=NORMAL;

    // get viewport coordinates in normalised units
    PLFLT ovpXL, ovpXR, ovpYB, ovpYT;
    gdlGetCurrentAxisWindow(XAXIS, ovpXL, ovpXR);
    gdlGetCurrentAxisWindow(YAXIS, ovpYB, ovpYT);
    //undefined or null previous viewport, seems IDL returns without complain:
    if ((ovpXL==ovpXR) || (ovpYB==ovpYT)) return;


    // old x and y range
    DDouble oxStart, oxEnd;
    DDouble oyStart, oyEnd;

    // get ![XY].CRANGE
    gdlGetCurrentAxisRange(XAXIS, oxStart, oxEnd, FALSE); //ignore projection limits, convert to linear values if necessary.
    gdlGetCurrentAxisRange(YAXIS, oyStart, oyEnd, FALSE);

    if ((oyStart == oyEnd) || (oxStart == oxEnd))
    {
      if (oyStart != 0.0 && oyStart == oyEnd){
        oyStart = 0;
        oyEnd = 1;
      }
      if (oxStart != 0.0 && oxStart == oxEnd){
        oxStart = 0;
        oxEnd = 1;
      }
    }

    PLFLT ovpSizeX, ovpSizeY;
    ovpSizeX=ovpXR-ovpXL;
    ovpSizeY=ovpYT-ovpYB;
    
    // new x and y range, real values (not log)
    DDouble xStart=oxStart;
    DDouble xEnd=oxEnd;
    DDouble yStart=oyStart;
    DDouble yEnd=oyEnd;

    // handle Log options passing via Keywords
    // note: undocumented keywords [xyz]type still exist and
    // have priority on [xyz]log !
    static int xTypeIx = e->KeywordIx( "XTYPE" );
    static int yTypeIx = e->KeywordIx( "YTYPE" );
    static int xLogIx = e->KeywordIx( "XLOG" );
    static int yLogIx = e->KeywordIx( "YLOG" );
    static int xTickunitsIx = e->KeywordIx( "XTICKUNITS" );
    static int yTickunitsIx = e->KeywordIx( "YTICKUNITS" );

    if ( e->KeywordPresent( xTypeIx ) ) xLog = e->KeywordSet( xTypeIx ); else xLog = e->KeywordSet( xLogIx );
    if ( e->KeywordPresent( yTypeIx ) ) yLog = e->KeywordSet( yTypeIx ); else yLog = e->KeywordSet( yLogIx );

    if ( xLog && e->KeywordSet( xTickunitsIx ) ) {
      Message( "PLOT: LOG setting ignored for Date/Time TICKUNITS." );
      xLog = FALSE;
    }
    if ( yLog && e->KeywordSet( yTickunitsIx ) ) {
      Message( "PLOT: LOG setting ignored for Date/Time TICKUNITS." );
      yLog = FALSE;
    }

    //XRANGE and YRANGE overrides all that, but  Start/End should be recomputed accordingly
    DDouble xAxisStart, xAxisEnd, yAxisStart, yAxisEnd;
    bool setx=gdlGetDesiredAxisRange(e, XAXIS, xAxisStart, xAxisEnd);
    bool sety=gdlGetDesiredAxisRange(e, YAXIS, yAxisStart, yAxisEnd);
    if (sety)
    {
      yStart=yAxisStart;
      yEnd=yAxisEnd;
    }
    if (setx)
    {
      xStart=xAxisStart;
      xEnd=xAxisEnd;
    }
    //handle Nozero option after all that!
    if(!gdlYaxisNoZero(e) && yStart >0 && !yLog ) yStart=0.0;

    gdlHandleUnwantedAxisValue(xStart, xEnd, xLog);
    gdlHandleUnwantedAxisValue(yStart, yEnd, yLog);

    // [XY]STYLE
    DLong xStyle=0, yStyle=0;
    gdlGetDesiredAxisStyle(e, XAXIS, xStyle);
    gdlGetDesiredAxisStyle(e, YAXIS, yStyle);

     //xStyle and yStyle apply on range values
    if ((xStyle & 1) != 1) {
      PLFLT intv = gdlAdjustAxisRange(e, XAXIS, xStart, xEnd, xLog);
    }
    if ((yStyle & 1) != 1) {
      PLFLT intv = gdlAdjustAxisRange(e, YAXIS, yStart, yEnd, yLog);
    }
    
    DDouble yVal, xVal;
    //in absence of arguments we will have:
    bool ynodef=true;
    bool xnodef=true;
    yVal=(standardNumPos)?oyStart:oyEnd;
    xVal=(standardNumPos)?oxStart:oxEnd;
    //read arguments 
    if (nParam() == 1) {
      e->AssureDoubleScalarPar( 0, xVal);
      xnodef=false;
    }
    if (nParam() == 2) {
      e->AssureDoubleScalarPar( 0, xVal);
      xnodef=false;
      e->AssureDoubleScalarPar( 1, yVal);
      ynodef=false;
    }
    if (nParam() == 3) {
      e->Throw( "Sorry, we do not yet support the 3D case");
    }

    // *** start drawing
    gdlSetGraphicsForegroundColorFromKw(e, actStream);       //COLOR
    //    contrary to the documentation axis does not erase the plot (fortunately!)
    //    gdlNextPlotHandlingNoEraseOption(e, actStream, true);     //NOERASE -- not supported

    PLFLT vpXL, vpXR, vpYB, vpYT; //define new viewport in relative units
    // where is point of world coords xVal, yVal in viewport relative coords?
    DDouble vpX,vpY;
    if ( coordinateSystem==DEVICE )
    {
      actStream->DeviceToNormedDevice(xVal, yVal, vpX, vpY);
    }
    else if ( coordinateSystem==NORMAL )
    {
      vpX=xVal;
      vpY=yVal;
    }
    else //DATA
    {

#ifdef USE_LIBPROJ
      // Map Stuff (xtype = 3)
      LPTYPE idata;
      XYTYPE odata;
      bool mapSet=false;
      get_mapset(mapSet);
      mapSet=(mapSet && coordinateSystem==DATA);
      if ( mapSet )
      {
        ref=map_init();
        if ( ref==NULL )
        {
          e->Throw("Projection initialization failed.");
        }
#if PROJ_VERSION_MAJOR >= 5
        idata.lam=xVal * DEG_TO_RAD;
        idata.phi=yVal * DEG_TO_RAD;
        odata=protect_proj_fwd_lp(idata, ref);
        xVal=odata.x;
        yVal=odata.y;
#else
        idata.u=xVal * DEG_TO_RAD;
        idata.v=yVal * DEG_TO_RAD;
        odata=PJ_FWD(idata, ref);
        xVal=odata.u;
        yVal=odata.v;
#endif
        DDouble *sx, *sy;
        GetSFromPlotStructs( &sx, &sy );

        DFloat *wx, *wy;
        GetWFromPlotStructs( &wx, &wy );

        DDouble pxStart, pxEnd, pyStart, pyEnd;
        DataCoordLimits( sx, sy, wx, wy, &pxStart, &pxEnd, &pyStart, &pyEnd, true );
        actStream->vpor( wx[0], wx[1], wy[0], wy[1] );
        actStream->wind( pxStart, pxEnd, pyStart, pyEnd );
      }
#endif
      if (xAxisWasLog) xVal=log10(xVal);
      if (yAxisWasLog) yVal=log10(yVal);
      if ( !isfinite(xVal)|| !isfinite(yVal) ) return; //no plot
      actStream->WorldToNormedDevice(xVal, yVal, vpX, vpY);
    }
    //compute new temporary viewport in relative coords
#define ADDEPSILON 0.1
    if ( standardNumPos )
    {
      vpXL=(xAxis || xnodef )?ovpXL       :vpX;
      vpXR=(xAxis || xnodef )?ovpXR       :vpX+ovpSizeY;
      vpYB=(yAxis || ynodef )?ovpYB       :vpY         ;
      vpYT=(yAxis || ynodef )?ovpYT       :vpY+ovpSizeX;
    }
    else
    {
      vpXL=(xAxis || xnodef )?ovpXL  :vpX-ovpSizeY;
      vpXR=(xAxis || xnodef )?ovpXR  :vpX;
      vpYB=(yAxis || ynodef )?ovpYB  :vpY-ovpSizeX;
      vpYT=(yAxis || ynodef )?ovpYT  :vpY;
    }
     
    actStream->OnePageSaveLayout(); // one page

    actStream->vpor(vpXL, vpXR, vpYB, vpYT);
    if (xLog) {xStart=log10(xStart);xEnd=log10(xEnd);}
    if (yLog) {yStart=log10(yStart);yEnd=log10(yEnd);}
    
    //insure 'wind' arguments are 
    actStream->wind(xStart, xEnd, yStart, yEnd);

    if ( xAxis )
    { //special ID "XAXIS2" needed because we artificially changed size of box
      gdlAxis(e, actStream, XAXIS2, xStart, xEnd, xLog, standardNumPos?1:2, ovpSizeY);

      if ( e->KeywordSet(SAVEIx) )
      {
        gdlStoreAxisCRANGE(XAXIS, xStart, xEnd, xLog);
        gdlStoreAxisType(XAXIS, xLog);
        gdlStoreAxisSandWINDOW(actStream,XAXIS, xStart, xEnd, xLog);
      }
    }

    if ( yAxis )
    {//special id "YAXIS2" needed because we artificially changed size of box
      gdlAxis(e, actStream, YAXIS2, yStart, yEnd, yLog, standardNumPos?1:2, ovpSizeX);

      if ( e->KeywordSet(SAVEIx) )
      {
        gdlStoreAxisCRANGE(YAXIS, yStart, yEnd, yLog);
        gdlStoreAxisType(YAXIS, yLog);
        gdlStoreAxisSandWINDOW(actStream,YAXIS, yStart, yEnd, yLog);
      }
    }
    // reset the viewport and world coordinates to the original values
    actStream->RestoreLayout();
    // if save, update world coordinates to the new values
    if ( e->KeywordSet(SAVEIx) )
    {
      actStream->wind(xStart, xEnd, yStart, yEnd);
    }

  }

    private: void call_plplot(EnvT* e, GDLGStream* actStream) 
    {
    } 
 
    private: virtual void post_call(EnvT* e, GDLGStream* actStream)
    {
       actStream->sizeChar(1.0);
    } 

  }; 

  void axis(EnvT* e)
  {
    axis_call axis;
    axis.call(e, 0);
  }

} // namespace
