/******************************************************************************
**                          Plush Version 1.0                                **
**         A portable high performance realtime 3D rendering library         **
**                                                                           **
**              All code copyright (c) 1996-1997, Justin Frankel             **
**               See the included Legal.doc for licensing terms              **
**                                                                           **
**                     Visit the Official Plush Page:                        **
**                  http://nullsoft.home.ml.org/plush.html                   **
**                                                                           **
**                  Send comments, improvements, etc to:                     **
**                          j.frankel@m.cc.utah.edu                          **
**                                                                           **
******************************************************************************/

/****
***** Configuration
****/

/* Type to use for general floating point operations: float or double */
#define PL_FloatType double

/* 16 bit integer */
#define PL_Int16Type short int

/* 32 bit integer */
#define PL_Int32Type long int

/* Type to use for zbuffer: must be floating point */
#define PL_ZBufferType float

/* MUST be 32 bit IEEE float */
#define PL_IEEEFloat32Type float

/* Maximum number of children that an object can have */
#define PL_MAX_CHILDREN (16)

/****
***** DEFINES
****/

#define plMin(x,y) (( ( x ) > ( y ) ? ( y ) : ( x )))
#define plMax(x,y) (( ( x ) < ( y ) ? ( y ) : ( x )))
#define PL_PI 3.14159265359

#define PL_SHADE_NONE (0x0)
#define PL_SHADE_FLAT (0x01)
#define PL_SHADE_GOURAUD (0x02)

#define PL_FILL_SOLID (0x0)
#define PL_FILL_TEXTURE (0x1)
#define PL_FILL_ENVIRONMENT (0x2)
#define PL_FILL_TRANSPARENT (0x4)

#define PL_LIGHT_NONE (0x0)
#define PL_LIGHT_VECTOR (0x1)
#define PL_LIGHT_POINT (0x2)

/****
***** TYPES
****/

typedef PL_ZBufferType pl_ZBufferType;
typedef PL_FloatType pl_FloatType;
typedef PL_IEEEFloat32Type pl_IEEEFloat32Type;
typedef signed PL_Int32Type pl_sInt32Type;
typedef unsigned PL_Int32Type pl_uInt32Type;
typedef signed PL_Int16Type pl_sInt16Type;
typedef unsigned PL_Int16Type pl_uInt16Type;

typedef struct _pl_TextureType {
  unsigned char *Data;         /* Texture data */
  unsigned char *PaletteData;  /* Palette data */
  unsigned char Height;        /* Log2 of width */
  unsigned char Width;         /* Log2 of height */
  unsigned char NumColors;     /* Number of colors used in texture */
} pl_TextureType;

typedef struct _pl_MaterialType {
  unsigned char NumGradients;  /* Desired number of gradients */
  unsigned char Priority;      /* Priority: 0 is highest, 255 lowest */
  unsigned char AmbientLight;  /* Ambient light value */
  pl_sInt16Type Red, Green, Blue; /* Diffuse RGB */
  pl_sInt16Type RedSpec, GreenSpec, BlueSpec; /* "Specular" RGB */
  pl_sInt16Type Shininess;     /* Shininess of material. 1 is dullest */
  pl_FloatType  EnvScaling;    /* Environment map scaling */
  unsigned char ShadeType;     /* Shade type: PL_SHADE_* */
  unsigned char Transparent;   /* Transparency index */
  unsigned char PerspectiveCorrect; /* Correct textures every n pixels */
  pl_TextureType *Texture;     /* Texture map */
  pl_TextureType *Environment; /* Env map */
  unsigned char _AddTable[256];
  unsigned char _st, _ft, _tsfact;
  unsigned char _ColorsUsed, _ColorBase;
} pl_MaterialType;

typedef struct _pl_VertexType {
  pl_FloatType x, y, z;        /* Vertex */
  pl_FloatType nx, ny, nz;     /* Vertex normal */
  pl_FloatType xformedx, xformedy, xformedz; /* Transformed vertex */
  pl_FloatType Shade;          /* Vertex intensity (for Gouraud shading) */
  pl_FloatType sLighting;      /* Static lighting. Should usually be 0.0 */
  pl_sInt32Type scrx, scry;     /* Screen coordinates of vertex (16.16) */
  pl_sInt32Type eMappingU, eMappingV; /* Environment map coordinates (16.16)*/
} pl_VertexType;

typedef struct _pl_FaceType {
  pl_FloatType nx, ny, nz;     /* Normal */
  pl_VertexType *Vertices[3];  /* Vertices of triangle */
  pl_MaterialType *Material;   /* Material of triangle */
  pl_sInt32Type MappingU[3], MappingV[3]; /* 16.16 Mapping coordinates */ 
  pl_FloatType Shade;          /* Face intensity (for flat shading) */
  pl_FloatType sLighting;      /* Static lighting. Should usually be 0.0 */
} pl_FaceType;

typedef struct _pl_ObjectType {
  pl_uInt32Type NumVertices;
  pl_uInt32Type NumFaces;
  pl_VertexType *Vertices;
  pl_FaceType *Faces;
  struct _pl_ObjectType *Children[PL_MAX_CHILDREN];
  unsigned char BackfaceCull;         /* Are backfacing polys drawn? */
  unsigned char BackfaceIllumination; /* Illuminated by lights behind them? */ 
  pl_FloatType Xp, Yp, Zp, Xa, Ya, Za;
} pl_ObjectType;

typedef struct _pl_SplineType {
  pl_FloatType *keys;              /* Key data, in chunks of numElem */
  pl_uInt32Type numElem;           
  pl_FloatType cont, bias, tens;   
  pl_FloatType t1,t2,t3,t4,u1,u2,u3,u4,v1,v2,v3; /* Internal */
} pl_SplineType;

typedef struct _pl_LightType {
  unsigned char Type;              /* PL_LIGHT_* */
  pl_FloatType Xp, Yp, Zp;         /* Position (PL_LIGHT_POINT),
                                      Unit vector (PL_LIGHT_VECTOR) */
  pl_FloatType Intensity;          /* 0.0 is off, 1.0 is full */
} pl_LightType;

typedef struct _pl_CameraType {
  pl_FloatType Fov;                /* FOV in degrees */
  pl_FloatType AspectRatio;        /* Aspect ratio */
  signed char Sort;                /* Sort polygons, -1 ftb, 1 btf, 0 no */
  unsigned char GouraudEnabled;    /* Gouraud shading enabled? */
  unsigned char EnvEnabled;        /* Environment mapping enabled? */
  pl_FloatType ClipFront, ClipBack; /* Depth clipping in camera space */
  pl_sInt32Type ClipTop, ClipLeft;  /* Clipping in screen space */
  pl_sInt32Type ClipBottom, ClipRight; 
  pl_sInt32Type ScreenWidth, ScreenHeight; /* Screen dimensions */
  pl_sInt32Type CenterX, CenterY;   /* Center of screen */
  pl_FloatType Xp, Yp, Zp;         /* Camera position */
  pl_FloatType Xa, Ya, Za;         /* Camera angle */
  unsigned char *frameBuffer;      /* Framebuffer */
  pl_ZBufferType *zBuffer;         /* Z Buffer (NULL if none) */
  pl_uInt16Type *_ScanLineBuffer;  /* Dirty scanlining buffer */
} pl_CameraType;

/****
***** VARIABLES
****/

extern char plVersionString[];                    /* Version string */
extern char plCopyrightString[];                  /* Copyright string */

/****
***** FUNCTIONS
****/

/**
*** material.c: material color ramp and table generation
**/

/*
** plInitializeMaterial() initializes the material "m", and if "g" is nonzero,
**   sets the appropriate palette entries in "pal". 
** Notes: Use plPM*() instead of this function, for most apps 
*/
void plInitializeMaterial(unsigned char *pal, pl_MaterialType *m, char g);

/** 
*** pm.c: palette managing
**/

/*
** plPMBegin() initializes the palette manager, to use "palette" from 
**   "begin" to "end".
*/
void plPMBegin(unsigned char *palette, unsigned char begin, unsigned char end);

/*
** plPMAddMaterial() adds "m" to the list of materials.
*/
void plPMAddMaterial(pl_MaterialType *m);

/*
** plPMEnd() finishes the palette management process, and actually creates 
**   the palette.
*/
void plPMEnd();

/** 
*** objutil.c: object utility functions
**/

/*
** plScaleObect() scales "o" (and all subobjects) by "s" and returns a 
**   pointer to "o".
**   Note: This is a slow routine, so don't use it every frame. ;)
*/
pl_ObjectType *plScaleObject(pl_ObjectType *o, pl_FloatType s);

/*
** plStretchObject() stretches "o" (and all subobjects) by "x","y","z". 
**   It recalculates the normals of "o" if "rcn" is nonzero. 
**   Returns a pointer to "o". 
**   Note: This is a slow routine, so don't use it every frame. ;)
*/
pl_ObjectType *plStretchObject(pl_ObjectType *o, pl_FloatType x,
                               pl_FloatType y, pl_FloatType z, char rcn);

/*
** plTranslateObject() translates "o" (and all subobjects) by "x","y","z", 
**   and returns "o".
**   Note: this is for pre-translations only. For realtime translations,
**         just use o->Xp, etc.
*/
pl_ObjectType *plTranslateObject(pl_ObjectType *o, pl_FloatType x,
                                 pl_FloatType y, pl_FloatType z);

/*
** plFlipObjectNormals() flips all vertex and face normals of "o" and all
**   subobjects of "o". 
*/

pl_ObjectType *plFlipObjectNormals(pl_ObjectType *o);

/*
** plFreeObject() frees "o" and all subobjects of "o".
*/
void plFreeObject(pl_ObjectType *o);

/* 
** plAllocObject() allocates an object with "np" vertices, "nf" faces,
**   and returns a pointer to it. 
*/
pl_ObjectType *plAllocObject(pl_uInt32Type np, pl_uInt32Type nf);

/* 
** plCloneObject() creates an exact but independent duplicate of "o" and all of
** "o"'s subobjects, and returns a pointer to it.
*/
pl_ObjectType *plCloneObject(pl_ObjectType *o);

/*
** plScaleObjectTexture() scales the texture coordinates of "o" 
**   (and all subobjects) by "u","v" and returns a pointer to "o".
*/
pl_ObjectType *plScaleObjectTexture(pl_ObjectType *o, pl_FloatType u,
                                    pl_FloatType v);

/*
** plSetObjectMaterial() sets the material of all faces in "o" to be "m",
**   and all subobjects if "th" is nonzero.
*/
void plSetObjectMaterial(pl_ObjectType *o, pl_MaterialType *m, char th);

/**
*** plush.c: more misc stuff!
**/

/*
** plNewLight() creates a new light and returns a pointer to it.
*/
pl_LightType *plNewLight();

/*
** plSetLight() sets up a light allocated with plNewLight(), and returns
**   a pointer to "light".
**   note: mode should be one of PL_LIGHT_*
*/
pl_LightType *plSetLight(pl_LightType *light, unsigned char mode,
                         pl_FloatType x, pl_FloatType y, 
                         pl_FloatType z, pl_FloatType intensity);

/*
** plFreeLight() frees "l".
*/
void plFreeLight(pl_LightType *l);

/*
** plPutFace() puts "face" to the screen using the appropriate rasterizer.
**   Note: really should not be used by itself, but only by the library.
*/
void plPutFace(pl_CameraType *cam, pl_FaceType *face);

/*
** plFreeTexture() frees all memory associated with "t"
*/
void plFreeTexture(pl_TextureType *t);

/* 
** plNewCamera() allocates a new camera and returns a pointer to it.
**  Parameters:
**   sw: screen width
**   sh: screen height
**   ar: aspect ratio (usually 1.0)
**   fov: field of view (usually 45-120)
**   ds: use dirty scanlining? (See plWriteBuffer())
**   fb: pointer to framebuffer
**   zb: pointer to Z buffer (or NULL)
**     note: if NULL, c->Sort is set, otherwise c->Sort is 0.
*/
pl_CameraType *plNewCamera(pl_sInt32Type sw, pl_sInt32Type sh, 
                           pl_FloatType ar, pl_FloatType fov,
                           unsigned char ds, unsigned char *fb,
                           pl_ZBufferType *zb);

/*
** plSetCameraTarget() sets the target of "c" to "x","y","z".
*/
void plSetCameraTarget(pl_CameraType *c, pl_FloatType x, pl_FloatType y,
                       pl_FloatType z);

/*
** plFreeCamera() frees all memory associated with "c", excluding framebuffers
**   and Z buffers
*/
void plFreeCamera(pl_CameraType *c);

/**
*** dscan.c: dirty scanlining
**/

/* 
** plInitScanLineBuffers() initializes the dirty-scanlining buffers for 
**   "Camera". 
*/
void plInitScanLineBuffers(pl_CameraType *Camera);

/*
** plFreeScanLineBuffers() frees the dirty-scanlining buffers for "Camera".
*/
void plFreeScanLineBuffers(pl_CameraType *Camera);

/*
** plWriteBuffer() writes the framebuffer of "Camera" out using dirty-scanling,
**   calling "moveout" and "movein". Not enough space for how this works here,
**   so look at some example source, or don't worry about it. ;)
*/
void plWriteBuffer(pl_CameraType *Camera,
                 void (*moveout)(pl_CameraType *,pl_uInt32Type,pl_uInt32Type),
                 void (*movein)(pl_CameraType *,pl_uInt32Type,pl_uInt32Type));

/**
*** render.c: the render manager!
**/

/*
** plRenderBegin() begins the rendering process, using "Camera".
**   Only one rendering process can occur at a time. 
*/
void plRenderBegin(pl_CameraType *Camera);

/*
** plRenderLight() adds "light" to the scene. Any objects rendered before
**   will be unaffected by this.
*/
void plRenderLight(pl_LightType *light);

/* 
** plRenderObject() adds "obj" and all subobjects to the scene.
*/
void plRenderObject(pl_ObjectType *obj);

/* 
** plRenderEnd() actually does the rendering.
*/
void plRenderEnd();

/**
*** make.c: primitves!
**/

/* 
** plMakePlane() makes a plane of "w" by "d", perpendicular to the y axis,
**   facing upward, centered at (0,0,0) and divides it into "res"x"res" pieces, 
**   and returns a pointer to the object. Each face is of material "m".
*/
pl_ObjectType *plMakePlane(pl_FloatType w, pl_FloatType d, pl_uInt16Type res,
                           pl_MaterialType *m);

/*
** plMakeBox() makes a box of "w"x"d"x"h", centered at (0,0,0), and returns
**   a pointer to the object. Each face is of material "m".
*/
pl_ObjectType *plMakeBox(pl_FloatType w, pl_FloatType d, pl_FloatType h,
                         pl_MaterialType *m);

/* 
** plMakeCone() makes a cone of radius "r" and height "h", centered at the 
**   origin, with "div" sides. The bottom is capped if "cap" is nonzero.
**   Each face is of material "m".
*/
pl_ObjectType *plMakeCone(pl_FloatType r, pl_FloatType h, unsigned char div, 
                          char cap, pl_MaterialType *m);

/*
** plMakeCylinder() makes a cylinder of radius "r" and height "h", centered at
**   the origin, with "div" sides. It caps the ends if "captop" and "capbottom"
**   are nonzero. Each face is of material "m".
*/
pl_ObjectType *plMakeCylinder(pl_FloatType r, pl_FloatType h,
                              unsigned char divr, char captop, char capbottom,
                              pl_MaterialType *m);

/*
** plMakeSphere() makes a sphere radius "r" centered at the origin.
**   It is divided up by "divr" and "divh" by radius and height respectively.
**   Each face is of material "m".
*/
pl_ObjectType *plMakeSphere(pl_FloatType r, unsigned char divr, 
                            unsigned char divh, pl_MaterialType *m);

/*
** plMakeTorus() makes a torus with inner radius "r1" outer radius "r2" 
**   centered at the origin. It is divided into "divrot" tubes, each divided
**   "divrad" times around. (Or something like that ;)
**   Each face is of material "m".
*/
pl_ObjectType *plMakeTorus(pl_FloatType r1, pl_FloatType r2,
                           unsigned char divrot, unsigned char divrad, 
                           pl_MaterialType *m);

/**
*** 3ds.c: the Autodesk .3DS reader
**/

/* 
** plReadObject3DS() reads a 3DS object hierarchy from "fn" and returns a 
**   pointer to the object. Each face is of material "m".
*/
pl_ObjectType *plReadObject3DS(char *fn, pl_MaterialType *m);

/**
*** cob.c: the ASCII .COB reader
**/

/*
** plReadObjectCOB() reads a ascii .COB object from "fn" and returns a pointer 
**   to it. Each face is of material "mat".
*/
pl_ObjectType *plReadObjectCOB(char *fn, pl_MaterialType *mat);

/**
*** jaw.c: the .JAW object reader
**/

/*
** plReadObjectJAW() reads a .JAW object from "fn" and returns a pointer to it.
**   Each face is of material "m".
*/
pl_ObjectType *plReadObjectJAW(char *fn, pl_MaterialType *m);

/**
*** pcx.c: the PCX texture reader.
**/

/*
** plReadTexturePCX() reads a 8bpp PCX file with power-of-two dimensions
**   from  "fn" and returns a pointer to it. 
*/
pl_TextureType *plReadTexturePCX(char *fn);

/** 
*** math.c: math code
**/

/*
** plMatrixRotate() generates a rotation matrix around the axis specified
**   by "m" by "Deg" degrees into "matrix". 
**   m: 1=X, 2=Y, 3=Z.
*/
void plMatrixRotate(pl_FloatType *matrix, unsigned char m, pl_FloatType Deg);

/*
** plMatrixTranslate() generates a translation matrix into "m"
*/
void plMatrixTranslate(pl_FloatType *m, pl_FloatType x, pl_FloatType y,
                       pl_FloatType z);

/* 
** plMatrixMultiply() multiplies "dest" by "src" and stores the result in "dest"
*/
void plMatrixMultiply(pl_FloatType *dest, pl_FloatType *src);

/*
** plMatrixApply() applies "m" to "x","y","z" and stores the result in
**   "outx", etc.
*/
void plMatrixApply(pl_FloatType *m, pl_FloatType x, pl_FloatType y, 
                   pl_FloatType z, pl_FloatType *outx, 
                   pl_FloatType *outy, pl_FloatType *outz);

/*
** plCalcNormals() calculates all face and vertex normals for "obj" (and all
**   subobjects)
*/
void plCalcNormals(pl_ObjectType *obj);

/*
** plNormalizeVector() makes the vector (x,y,z) a unit vector
*/
void plNormalizeVector(pl_FloatType *x, pl_FloatType *y, pl_FloatType *z);

/*
** plDotProduct() returns the dot product of (x1,y1,z1) and (x2,y2,z2)
*/
pl_FloatType plDotProduct(pl_FloatType x1, pl_FloatType y1, pl_FloatType z1,
                          pl_FloatType x2, pl_FloatType y2, pl_FloatType z2);

/**
*** spline.c: spline interpolation functions!
**/

/*
** plSplineInit() initializes "s".
*/
void plSplineInit(pl_SplineType *s);

/*
** plSplineGetPoint() gets the point from the spline "s" at time "frame",
**   into "out". (Look at some example source for how to use this)
*/
void plSplineGetPoint(pl_SplineType *s, pl_FloatType frame, pl_FloatType *out);

/**
*** ?f_*.c: rasterizers! 
*** zf_*.c: zbuffering
*** pf_*.c: non-zbuffering
**/

/*
** non-ZBuffering fillers
*/
void plPF_SolidF(pl_CameraType *, pl_FaceType *);
void plPF_SolidG(pl_CameraType *, pl_FaceType *);
void plPF_TexF(pl_CameraType *, pl_FaceType *);
void plPF_TexG(pl_CameraType *, pl_FaceType *);
void plPF_EnvF(pl_CameraType *, pl_FaceType *);
void plPF_EnvG(pl_CameraType *, pl_FaceType *);
void plPF_TexEnv(pl_CameraType *, pl_FaceType *);
void plPF_PTexF(pl_CameraType *, pl_FaceType *);
void plPF_PTexG(pl_CameraType *, pl_FaceType *);
void plPF_TransF(pl_CameraType *, pl_FaceType *);
void plPF_TransG(pl_CameraType *, pl_FaceType *);
/*
** ZBuffering fillers
*/ 
void plPF_SolidFZB(pl_CameraType *, pl_FaceType *);
void plPF_SolidGZB(pl_CameraType *, pl_FaceType *);
void plPF_TexFZB(pl_CameraType *, pl_FaceType *);
void plPF_TexGZB(pl_CameraType *, pl_FaceType *);
void plPF_EnvFZB(pl_CameraType *, pl_FaceType *);
void plPF_EnvGZB(pl_CameraType *, pl_FaceType *);
void plPF_TexEnvZB(pl_CameraType *, pl_FaceType *);
void plPF_PTexFZB(pl_CameraType *, pl_FaceType *);
void plPF_PTexGZB(pl_CameraType *, pl_FaceType *);
void plPF_TransFZB(pl_CameraType *, pl_FaceType *);
void plPF_TransGZB(pl_CameraType *, pl_FaceType *);

/* END */
