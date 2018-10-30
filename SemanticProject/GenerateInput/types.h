#ifndef TYPES_H
#define TYPES_H

#include <opencv2/opencv.hpp>

////////////////////////////////////////////////////////////////////////////
/// \brief dsvlprocessor
///////////////////////////////

const int maxn = 2000000000;
const double topi = acos(-1.0)/180.0;	// pi/180
#define BOUND(x,min,max) ((x) < (min) ? (min) : ((x) > (max) ? (max) : (x)))
#define	nint(x)			(int)((x>0)?(x+0.5):(x-0.5))
#define	sqr(x)			((x)*(x))

struct point2d
{
    double x;
    double y;
};

struct point3d
{
    double x;
    double y;
    double z;
};

typedef double  MATRIX[3][3] ;

#define	PNTS_PER_LINE		32
#define	LINES_PER_BLK		12
#define	PTNUM_PER_BLK		(32*12)
#define	BKNUM_PER_FRM		180
#define	SCANDATASIZE		(180*12)

#define	M_PI		3.1415926536

#define	INVALIDDOUBLE		99999999.9


typedef struct {
    float			x, y, z;
    u_char			i;
} point3fi;

typedef struct {
    int x, y;
} point2i;

typedef struct {
    int			millisec;
    point3fi		points[PTNUM_PER_BLK];
} ONEVDNDATA;

typedef struct {
    point3d			ang;
    point3d			shv;
    int			millisec;
    point3fi		points[PTNUM_PER_BLK];
    MATRIX			rot;
    int				lab[PTNUM_PER_BLK];
} ONEDSVDATA;

typedef struct {
    ONEDSVDATA		dsv[BKNUM_PER_FRM];
    point3d			ang;
    point3d			shv;
    MATRIX			rot;
} ONEDSVFRAME;


typedef struct {
    int				wid;
    int				len;
    point3fi		*pts;
    int				*regionID;
    int				regnum;
    IplImage		*rMap;
    IplImage		*lMap;
    point3d			ang;
    point3d			shv;
    int			millsec;
} RMAP;

#define	INVAFLG			0
#define	LINEFLG			10
#define	NODEFLG			11
#define	SEGFLG			12
#define	ROADFLG			13
#define	HIGHFLG			14

#define	OBGLAB0			0
#define	OBGLAB1			1
#define	OBGLAB2			2
#define	OBGLAB3			3
#define	OBGLAB4			4
#define	OBGLAB5			5
#define	OBGLAB6			6
#define	OBGLAB7			7
#define OBJGROUND		100
#define	OBJBACKGROUND	101


#define UNKNOWN			0
#define NONVALID		-9999
#define GROUND			-999
#define	BACKGROUND		-99
#define EDGEPT			-9
#define	CHECKEDLABEL	-7

//////////////////////////////////////////////////////
///samplegenerator
/////////////////////////////////////////////////////
///


struct LABELLEDPONTS
{
    int img_x, img_y;
    cv::Point3f loc;
    int intensity;
};

typedef struct IDTYPE {
    int id;
    int point_num;
    cv::Point center;
    std::string name;

    IDTYPE() {
        id = point_num = 0;
    }
    IDTYPE(int id_, int point_num_) {
        id = id_;
        point_num = point_num_;
        name = "";
    }

    //std::sort
    bool operator()(IDTYPE left, IDTYPE right) { return left.id < right.id; }

    //std::unique
    bool operator==(const IDTYPE& rhs) { return this->id == rhs.id; }
}IDNUM;

//////////////////////////////////////////////////
/// others
/////////////////////////////////////////////////

#define	COLORNUM		16
static const unsigned char	LEGENDCOLORS[COLORNUM][3] =
            {
                {128,255,255},{0,0,255},{255,0,0},{0,255,0},
                {128,128,255},{255,255,0},{0,128,255},{255,0,255},{0,255,255},
                {128,0,128},{255,128,0},{255,128,255},
                {164,255,255},
                {255,64,64},{0,128,128},{128,128,0},

            };
#endif // TYPES_H
