typedef union{
  double val;
  char string[255];

} YYSTYPE;
#define	COMMENT	258
#define	DOUBLE	259
#define	LIGHT	260
#define	AMBIENT	261
#define	CONSTANTS	262
#define	SAVE_COORDS	263
#define	CAMERA	264
#define	SPHERE	265
#define	TORUS	266
#define	BOX	267
#define	LINE	268
#define	CS	269
#define	MESH	270
#define	TEXTURE	271
#define	STRING	272
#define	SET	273
#define	MOVE	274
#define	SCALE	275
#define	ROTATE	276
#define	BASENAME	277
#define	SAVE_KNOBS	278
#define	TWEEN	279
#define	FRAMES	280
#define	VARY	281
#define	PUSH	282
#define	POP	283
#define	SAVE	284
#define	GENERATE_RAYFILES	285
#define	SHADING	286
#define	SHADING_TYPE	287
#define	SETKNOBS	288
#define	FOCAL	289
#define	DISPLAY	290
#define	WEB	291
#define	CO	292


extern YYSTYPE yylval;
