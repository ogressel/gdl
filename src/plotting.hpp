/* *************************************************************************
   plotting.hpp  -  GDL routines for plotting
   -------------------
   begin                : July 22 2002
   copyright            : (C) 2002 by Marc Schellens
   email                : m_schellens@users.sf.net
 ***************************************************************************/

/* *************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef PLOTTING_HPP_
#define PLOTTING_HPP_

//for 3D transformations, with "our" plplot drivers.
#define PLESC_2D 99
#define PLESC_3D 100

//To debug Affine 3D homogenous projections matrices.
//IDL define a matrix as  M[ncol,mrow] and print as such. However col_major and
//row_major refer to the math notation M[row,col] where row=dim(0) and col=dim(1).
//Matrices are stored COL Major in IDL/Fortran and ROW Major in C,C++ etc.
//so element at (i,j) is computed as  (j*dim0 + i) for ColMajor/IDL
//and (i*dim1 + j) for RowMajor/C

#define TRACEMATRIX_C(var__)      \
  {int dim0__=(var__)->Dim(0), dim1__=(var__)->Dim(1);   \
    fprintf(stderr,"c matrix[%d,%d]\n",dim0__,dim1__);   \
    for (int row=0; row < dim0__ ; row++)    \
      {         \
  for (int col=0; col < dim1__-1; col++)    \
          {        \
            fprintf(stderr,"%g, ",(*var__)[row*dim1__ + col]);  \
          }        \
  fprintf(stderr,"%g\n",(*var__)[row*dim1__ + dim1__ -1]); \
      }         \
    fprintf(stderr,"\n");      \
  }
//The following abbrevs should output the C matrix as IDL would do (ie,transposed):
#define TRACEMATRIX_IDL(var__)      \
  {int dim0__=(var__)->Dim(0), dim1__=(var__)->Dim(1);   \
    fprintf(stderr,"idl matrix[%d,%d]\n[",dim0__,dim1__);  \
    for (int col=0; col < dim1__; col++)    \
      {         \
  fprintf(stderr,"[");      \
  for (int row=0; row < dim0__; row++)    \
          {        \
            fprintf(stderr,"%g",(*var__)[row*dim1__ + col]);  \
            if (row<dim0__-1) fprintf(stderr," ,");   \
            else if (col<dim1__-1) fprintf(stderr," ],$\n"); else fprintf(stderr," ]]\n") ; \
          }        \
      }         \
  }

#include "envt.hpp"
#include "graphicsdevice.hpp"
#include "initsysvar.hpp"

#ifdef USE_LIBPROJ
#include "projections.hpp"
#endif 

#undef MIN
#define MIN(a,b) ((a) > (b) ? (b) : (a))
#undef MAX
#define MAX(a,b) ((a) < (b) ? (b) : (a))
#undef ABS
#define ABS(a) (((a) < 0) ? -(a) : (a))

typedef enum {
  DATA = 0,
  NORMAL,
  DEVICE,
  NONE //for TV()
} COORDSYS;

typedef struct {
  DDouble zValue;
  DDouble T[16];
} GDL_3DTRANSFORMDEVICE;

//for clipping, shared, defined in plotting_plot.cpp
extern PLFLT clipBoxInMemory[4];
extern COORDSYS coordinateSystemInMemory;

enum T3DEXCHANGECODE {
  INVALID = -1,
  NORMAL3D = 0,
  XY,
  XZ,
  YZ,
};

// to be removed:
#define  SCALEBYDEFAULT 1./sqrt(3) 

static const DDouble ZVALUEMAX=0.99998999;

enum PLOT_AXES_IDENTIFIERS {
  XAXIS = 0,
  YAXIS,
  ZAXIS,
};

enum PLOT_WHERE_DRAW_AXES {
  AT_BOTH = 0,
  AT_BOTTOM = 1,
  AT_TOP = 2,
};

static const std::string axisName[6] = {"X", "Y", "Z", "X", "Y", "Z"};
#define DRAWAXIS "a"     //: Draw axis (X is horizontal line Y=0, Y is vertical line X=0)
#define BOTTOM "b"     //: Draw bottom (X) or left (Y) frame of box
#define TOP "c"     //: Draw top (X) or right (Y) frame of box
//#define "d"     //: Interpret axis as a date/time when writing labels
//#define "f"     //: Always use fixed point numeric labels
//#define "g"     //: Draws a grid at the major tick interval
//#define "h"     //: Draws a grid at the minor tick interval
#define TICKINVERT "i"     //: Inverts tick marks
#define LOG "l"     //: Logarithmic axes, major ticks at decades, minor ticks at units
#define NUMERIC "n"     //: Write numeric label at conventional location
#define NUMERIC_UNCONVENTIONAL "m"     //: Write numeric label at unconventional location
#define LABELFUNC "o"     //: Label text is generated by a user-defined function
#define SUBTICKS "s"     //: Draw minor tick marks
#define TICKS "t"     //: Draw major tick marks
#define BOTTOM_NOLINE "u"     //: like b (including all side effects such as tick marks and numerical
// labels for those) except exclude drawing the edge.
#define TOP_NOLINE "w"     //: like c (including all side effects such as tick marks and numerical
// labels for those) except exclude drawing the edge.
#define YLABEL_HORIZONTAL "v"     //: (for Y only) Label vertically
#define NOTICKS "x"     //: like t (including the side effect of the numerical labels for the major
// ticks) except exclude drawing the major and minor tick marks.

#define GDL_NONE -1
#define GDL_TICKUNITS 1
#define GDL_TICKUNITS_AND_FORMAT 2

struct GDL_TICKDATA {
  GDLGStream *a;
  EnvT *e;
  // For TICKNAMES
  SizeT tickNameCounter; //internal counter of what tickname we use
  SizeT nTickName; //number of tickname values passed
  DStringGDL* TickName; //pointer to ticknames
  //For TICKFORMAT
  SizeT nTickFormat;
  DStringGDL* TickFormat; 
  // for TICKUNITS => Multi-axis 
  SizeT nTickUnits;
  DStringGDL* TickUnits;
  SizeT counter;
  DDouble Start; //used where the span of values in axis matters.
  DDouble End; //used where the spand of values in axis matters.
  double nchars; //length of string *returned* after formatting. Can be non-integer.
  int tickOptionCode;
  int tickLayoutCode;
  int maxPrec;
  bool isLog;
  bool reset; //reset internal counter each time a new 'axis' command is issued
};


namespace lib {

  using namespace std;

  // main plotting routine (all defined using the plotting_routine_call class)
  void plot(EnvT* e);
  void plot_io(EnvT* e);
  void plot_oo(EnvT* e);
  void plot_oi(EnvT* e);
  void oplot(EnvT* e);
  void plots(EnvT* e);
  void surface(EnvT* e);
  void shade_surf(EnvT* e);
  void contour(EnvT* e);
  void xyouts(EnvT* e);
  void axis(EnvT* e);
  void polyfill(EnvT* e);
  void tv_image(EnvT* e);
  void usersym(EnvT* e);
  void set_shading(EnvT* e);

  // other plotting routines
  void erase(EnvT* e);
  void tvlct(EnvT* e);
  void wshow(EnvT* e);
  void wdelete(EnvT* e);
  void wset(EnvT* e);
  void window(EnvT* e);
  void set_plot(EnvT* e);
  BaseGDL* get_screen_size(EnvT* e);
  void device(EnvT* e);
  void cursor(EnvT* e);
  void tvcrs(EnvT* e);
  void empty(EnvT* e);
  BaseGDL* format_axis_values(EnvT *e);
  void scale3_pro(EnvT* e);
  void t3d_pro(EnvT* e);

  BaseGDL* convert_coord(EnvT* e);

  // Map stuff
  void get_mapset(bool &mapset);
  void set_mapset(bool mapset);
#ifdef USE_LIBPROJ
  void GDLgrProjectedPolygonPlot(GDLGStream * a, PROJTYPE ref, DStructGDL* map, DDoubleGDL *lons, DDoubleGDL *lats, bool isRadians, bool const doFill, bool const doLines, DLongGDL *conn = NULL);
  DDoubleGDL* GDLgrGetProjectPolygon(GDLGStream * a, PROJTYPE ref, DStructGDL* map, DDoubleGDL *lons, DDoubleGDL *lats, DDoubleGDL *zVal, bool isRadians, bool const doFill, bool const dolines, DLongGDL *&conn);
#endif
  //3D conversions
  void SelfTranspose3d(DDoubleGDL* me);
  void SelfReset3d(DDoubleGDL* me);
  void SelfTranslate3d(DDoubleGDL* me, DDouble *trans);
  void SelfScale3d(DDoubleGDL* me, DDouble *scale);
  void SelfRotate3d(DDoubleGDL* me, DDouble *rot);
  void SelfPerspective3d(DDoubleGDL* me, DDouble zdist);
  void SelfOblique3d(DDoubleGDL* me, DDouble dist, DDouble angle);
  void SelfExch3d(DDoubleGDL* me, T3DEXCHANGECODE axisExchangeCode);
  void SelfProjectXY(DDoubleGDL *x, DDoubleGDL *y);
  void SelfProjectXY(SizeT nEl, DDouble *x, DDouble *y, COORDSYS const coordinateSystem);
  void yzaxisExch(DDouble* me);
  void yaxisFlip(DDouble* me);
  void SelfConvertToNormXYZ(DDoubleGDL* x, bool &xLog, DDoubleGDL* y, bool &yLog, DDoubleGDL* z, bool &zLog, COORDSYS &code);
  void SelfConvertToNormXYZ(DDouble &x, bool const xLog, DDouble &y, bool const yLog, DDouble &z, bool const zLog, COORDSYS const code);
  void SelfConvertToNormXY(SizeT n, PLFLT *xt, bool const xLog, PLFLT *yt, bool const yLog, COORDSYS const code);
  void SelfConvertToNormXY(DDoubleGDL* x, bool &xLog, DDoubleGDL* y, bool &yLog, COORDSYS &code);
  void SelfPDotTTransformXYZ(SizeT n, PLFLT *xt, PLFLT *yt, PLFLT *zt);
  void SelfPDotTTransformXYZ(DDoubleGDL *xt, DDoubleGDL *yt, DDoubleGDL *zt);
  void PDotTTransformXYZval(PLFLT x, PLFLT y, PLFLT *xt, PLFLT *yt, PLPointer data);
  DDoubleGDL* gdlDefinePlplotRotationMatrix(DDouble az, DDouble alt, DDouble *scale, bool save);
  void gdlMakeSubpageRotationMatrix3d(DDoubleGDL* me, PLFLT xratio, PLFLT yratio, PLFLT zratio, PLFLT* trans);
  void gdlMakeSubpageRotationMatrix2d(DDoubleGDL* me, PLFLT xratio, PLFLT yratio, PLFLT zratio, PLFLT* trans, PLFLT shift=0, bool invert=false);
  bool gdlInterpretT3DMatrixAsPlplotRotationMatrix(DDouble &az, DDouble &alt, DDouble &ay, DDouble *scale, /* DDouble *trans,*/  T3DEXCHANGECODE &axisExchangeCode, bool &below);
  DDoubleGDL* gdlGetT3DMatrix();
  void get3DMatrixParametersFor2DPosition(PLFLT &xratio, PLFLT &yratio, PLFLT &zratio, PLFLT* displacement);
  void gdlStartT3DMatrixDriverTransform(GDLGStream *a, DDouble zValue);
  void gdlStartSpecial3DDriverTransform( GDLGStream *a, GDL_3DTRANSFORMDEVICE &PlotDevice3D);
  void gdlExchange3DDriverTransform( GDLGStream *a);
  void gdlFlipYPlotDirection( GDLGStream *a);
  void gdlShiftYaxisUsing3DDriverTransform( GDLGStream *a, DDouble yval, bool invert=false);
  void gdlSetZto3DDriverTransform( GDLGStream *a, DDouble zValue);
  void gdlStop3DDriverTransform(GDLGStream *a);
  void Matrix3DTransformXYZval(DDouble x, DDouble y, DDouble z, DDouble *xt, DDouble *yt, DDouble *t);
  bool T3Denabled();
  void gdlDoRangeExtrema(DDoubleGDL *xVal, DDoubleGDL *yVal, DDouble &min, DDouble &max, DDouble xmin, DDouble xmax, bool doMinMax = false, DDouble minVal = 0, DDouble maxVal = 0);
  void draw_polyline(GDLGStream *a, DDoubleGDL *xVal, DDoubleGDL *yVal, DLong psym = 0, bool append = false, DLongGDL *color = NULL);
  void SelfNormLonLat(DDoubleGDL *lonlat);
  void SelfPDotTTransformProjectedPolygonTable(DDoubleGDL *lonlat);
  void GDLgrPlotProjectedPolygon(GDLGStream * a, DDoubleGDL *lonlat, bool const doFill, DLongGDL *conn);
  void gdlSetGraphicsPenColorToBackground(GDLGStream *a);
  void gdlLineStyle(GDLGStream *a, DLong style);
  PLFLT* gdlGetRegion();
  PLFLT gdlGetBoxNXSize(); // what !X thinks the current PLOT box size is, in normalised coordinates. To be used instead of the boxnXSize() gdlgstream function whenever possible.
  PLFLT gdlGetBoxNYSize(); // what !Y thinks the current PLOT box size is, in normalised coordinates. To be used instead of the boxnYSize() gdlgstream function whenever possible.
  DFloat* gdlGetWindow();
  void gdlStoreXAxisRegion(GDLGStream* actStream, PLFLT* r);
  void gdlStoreYAxisRegion(GDLGStream* actStream, PLFLT* r);
  void gdlStoreZAxisRegion(GDLGStream* actStream, PLFLT* r);
  void gdlStoreXAxisParameters(GDLGStream* actStream, DDouble Start, DDouble End, bool log);
  void gdlStoreYAxisParameters(GDLGStream* actStream, DDouble Start, DDouble End, bool log);
  void gdlStoreZAxisParameters(GDLGStream* actStream, DDouble Start, DDouble End, bool log, DDouble zposStart, DDouble zposEnd);
  void gdlGetAxisType(int axisId, bool &log);
  void gdlGetCurrentAxisWindow(int axisId, DDouble &wStart, DDouble &wEnd);
  void gdlStoreAxisType(int axisId, bool type);
  //  void gdlGetCharSizes(GDLGStream *a, PLFLT &nsx, PLFLT &nsy, DDouble &wsx, DDouble &wsy, 
  //		       DDouble &dsx, DDouble &dsy, DDouble &lsx, DDouble &lsy); 
  void GetSFromPlotStructs(DDouble **sx, DDouble **sy, DDouble **sz = NULL);
  void GetWFromPlotStructs(DDouble *wx, DDouble *wy, DDouble *wz = NULL);
  void ConvertToNormXY(SizeT n, DDouble *x, bool const xLog, DDouble *y, bool const yLog, COORDSYS const code);
  void ConvertToNormZ(SizeT n, DDouble *z, bool const zLog, COORDSYS const code);
  void gdlStoreCLIP();
//  void gdlGetCLIPXY(DLong &x0, DLong &y0, DLong &x1, DLong &y1);
  void GetCurrentUserLimits(DDouble &xStart, DDouble &xEnd, DDouble &yStart, DDouble &yEnd); //2D
  void GetCurrentUserLimits(DDouble &xStart, DDouble &xEnd, DDouble &yStart, DDouble &yEnd, DDouble &zStart, DDouble &zEnd); //3D
  void gdlAdjustAxisRange(EnvT* e, int axisId, DDouble &val_min, DDouble &val_max, bool &log);
  PLFLT AutoTickIntv(DDouble x, bool freeRange=false);
  PLFLT AutoLogTickIntv(DDouble min, DDouble max);
  void setIsoPort(GDLGStream* actStream, PLFLT x1, PLFLT x2, PLFLT y1, PLFLT y2, PLFLT aspect);
  void GetMinMaxVal(DDoubleGDL* val, double* minVal, double* maxVal);
  void GetMinMaxValuesForSubset(DDoubleGDL* val, DDouble &minVal, DDouble &maxVal, SizeT endElement);
  void UpdateSWPlotStructs(GDLGStream* actStream, DDouble xStart, DDouble xEnd, DDouble yStart,
    DDouble yEnd, bool xLog, bool yLog);
  DDoubleGDL* getLabelingValues(int axisId);
  DDoubleGDL* computeLabelingValues(int axisId);
  void defineLabeling(GDLGStream *a, int axisId, void(*func)(PLINT axis, PLFLT value, char *label, PLINT length, PLPointer data), PLPointer data);
  void resetLabeling(GDLGStream *a, int axisId);
  void gdlSimpleAxisTickFunc(PLINT axis, PLFLT value, char *label, PLINT length, PLPointer data);
  void gdlSingleAxisTickNamedFunc(PLINT axis, PLFLT value, char *label, PLINT length, PLPointer data);
  void gdlMultiAxisTickFunc(PLINT axis, PLFLT value, char *label, PLINT length, PLPointer data);
  void doOurOwnFormat(PLINT axisNotUsed, PLFLT value, char *label, PLINT length, PLPointer data);
  void gdlHandleUnwantedLogAxisValue(DDouble &min, DDouble &max, bool log);
  
  class plotting_routine_call {
    // ensure execution of child-class destructors
  public:

    virtual ~plotting_routine_call() {
    };

    // private fields
  private:
    SizeT _nParam;
  private:
    bool abort;
    //  private: bool isDB; //see below why commented.

    // common helper methods
  protected:

    inline SizeT nParam() {
      return _nParam;
    }

    // prototypes for methods defining various steps
  private:
    virtual bool handle_args(EnvT*) = 0; // return value = overplot
    virtual bool prepareDrawArea(EnvT*, GDLGStream*) = 0;
    virtual void applyGraphics(EnvT*, GDLGStream*) = 0;  //Should *NOT* contain any 'save' commands able to change !X,!Y,!Z,!P,!D values !!!! 
    virtual void post_call(EnvT*, GDLGStream*) = 0;

    // all steps combined (virtual methods cannot be called from ctor)
  public:
    void call(EnvT* e, SizeT n_params_required);
  };
 //
  //--------------FOLLOWING ARE STATIC FUNCTIONS-----------------------------------------------
  //This because static pointers to options indexes are needed to speed up process, but these indexes vary between
  //the definition of the caller functions (e.g. "CHARSIZE" is 1 for CONTOUR but 7 for XYOUTS). So they need to be kept
  //static (for speed) but private for each graphic command.
  void restoreDrawArea(GDLGStream *a);

  void gdlSetGraphicsBackgroundColorFromKw(EnvT *e, GDLGStream *a, bool kw = true);

   void gdlSetGraphicsForegroundColorFromBackgroundKw(EnvT *e, GDLGStream *a, bool kw = true);
  
  void gdlSetGraphicsForegroundColorFromKw(EnvT *e, GDLGStream *a, string OtherColorKw = "");

  void gdlGetPsym(EnvT *e, DLong &psym);

  void gdlSetSymsize(EnvT *e, GDLGStream *a);

  void gdlSetPlotCharsize(EnvT *e, GDLGStream *a, PLFLT use_factor = 1, bool accept_sizeKw = false);

  void gdlSetPlotCharthick(EnvT *e, GDLGStream *a);
  
  PLFLT gdlComputeAxisTickInterval(EnvT *e, int axisId, DDouble min, DDouble max, bool log, int level=0, bool freeRange=false);

  void gdlGetDesiredAxisCharsize(EnvT* e, int axisId, DFloat &charsize);

  void gdlSetAxisCharsize(EnvT *e, GDLGStream *a, int axisId);

  void gdlGetDesiredAxisGridStyle(EnvT* e, int axisId, DLong &axisGridstyle);

  void gdlGetDesiredAxisMargin(EnvT *e, int axisId, DFloat &start, DFloat &end);

  void gdlGetDesiredAxisMinor(EnvT* e, int axisId, DLong &axisMinor);

  bool gdlGetDesiredAxisRange(EnvT *e, int axisId, DDouble &start, DDouble &end);

  void gdlGetDesiredAxisStyle(EnvT *e, int axisId, DLong &style);

  void gdlGetDesiredAxisThick(EnvT *e, int axisId, DFloat &thick);

  void gdlGetDesiredAxisTickFormat(EnvT* e, int axisId, DStringGDL* &axisTickformatVect);

  void gdlGetDesiredAxisTickInterval(EnvT* e, int axisId, DDouble &axisTickinterval);

  void gdlGetDesiredAxisTickLayout(EnvT* e, int axisId, DLong &axisTicklayout);

  void gdlGetDesiredAxisTickLen(EnvT* e, int axisId, DFloat &ticklen);

  void gdlGetDesiredAxisTickName(EnvT* e, GDLGStream* a, int axisId, DStringGDL* &axisTicknameVect);

  void gdlGetDesiredAxisTicks(EnvT* e, int axisId, DLong &axisTicks);

  int gdlGetCalendarCode(EnvT* e, int axisId, int level=0);

  void gdlGetDesiredAxisTickUnits(EnvT* e, int axisId, DStringGDL* &axisTickunitsVect);
  bool gdlHasTickUnits(EnvT* e, int axisId);

  bool gdlGetDesiredAxisTickv(EnvT* e, int axisId, DDoubleGDL* &axisTickvVect);

  //if [X|Y|Z]TICK_GET was given for axis, write the values.

  void gdlGetDesiredAxisTickGet(EnvT* e, int axisId, DDouble TickInterval, DDouble Start, DDouble End, bool isLog);

  void gdlGetDesiredAxisTitle(EnvT *e, int axisId, DString &title);

  void gdlSetLineStyle(EnvT *e, GDLGStream *a);

  DFloat gdlGetPenThickness(EnvT *e, GDLGStream *a);

  void gdlSetPenThickness(EnvT *e, GDLGStream *a);

  //call this function if Y data is strictly >0.
  //set yStart to 0 only if gdlYaxisNoZero is false.

  bool gdlYaxisNoZero(EnvT* e);


  //advance to next plot unless the noerase flag is set
  // function declared static (local to each function using it) to avoid messing the NOERASEIx index which is not the same.

  void gdlNextPlotHandlingNoEraseOption(EnvT *e, GDLGStream *a);

  //handling of Z bounds is not complete IMHO.

  inline void CheckMargin(GDLGStream* actStream,
    DFloat xMarginL,
    DFloat xMarginR,
    DFloat yMarginB,
    DFloat yMarginT,
    PLFLT& xMR,
    PLFLT& xML,
    PLFLT& yMB,
    PLFLT& yMT);
  
  DDouble gdlSetViewPortAndWorldCoordinates(EnvT* e,
    GDLGStream* actStream,
    DDouble x0,
    DDouble x1,
    bool xLog,
    DDouble y0,
    DDouble y1,
    bool yLog,
    DDouble z0,
    DDouble z1,
    bool zLog,
    DDouble zValue_input, //input
    bool iso = false) ;
  
  //gdlAxis will write an axis from Start to End, following all modifier switch, in the good place of the current VPOR, independent of the current WIN,
  // as it is temporarily superseded by setting a new a->win().
  //this makes GDLAXIS independent of WIN, and help the whole code to be dependent only on VPOR which is the sole useful plplot command to really use.
  //ZAXIS will always be an YAXIS plotted with a special YZEXCH T3D matrix. So no special handling of ZAXIS here.

  void gdlAxis(EnvT *e, GDLGStream *a, int axisId, DDouble Start, DDouble End, bool Log, DLong modifierCode = 0);

  void gdlBox(EnvT *e, GDLGStream *a, DDouble xStart, DDouble xEnd, bool xLog, DDouble yStart, DDouble yEnd, bool yLog);
  
  void gdlBox3(EnvT *e, GDLGStream *a, DDouble xStart, DDouble xEnd, bool xLog, DDouble yStart, DDouble yEnd, bool yLog,  DDouble zStart, DDouble zEnd, bool zLog, DDouble zValue, bool drawZ=false, DLong zcode=0);
   
  //restore current clipbox, make another or remove it at all.
  bool gdlSwitchToClippedNormalizedCoordinates(EnvT *e, GDLGStream *actStream, bool invertedClipMeaning=false, bool commandHasCoordSys=true );

  
  //just test if clip values (!P.CLIP , CLIP=) are OK (accounting for all NOCLIP etc possibilities!)
  bool gdlTestClipValidity(EnvT *e, GDLGStream *actStream, bool invertedClipMeaning=false, bool commandHasCoordSys=true );

} // namespace

#endif
