// 19 april 2019

#ifdef __cplusplus
extern "C" {
#endif

// init.c
extern const char **uiprivSysInitErrors(void);
extern int uiprivSysInit(void *options, uiInitError *err);
extern int uiprivInitReturnError(uiInitError *err, const char *msg);
extern int uiprivInitReturnErrorf(uiInitError *err, const char *msg, ...);

// programmererror.c
enum {
	uiprivProgrammerErrorWrongStructSize,		// arguments: size_t badSize, const char *structName
	uiprivProgrammerErrorIndexOutOfRange,	// arguments: int badIndex, __func__
	uiprivNumProgrammerErrors,
};
extern void uiprivProgrammerError(unsigned int which, ...);
extern void uiprivSysProgrammerError(const char *msg);
#define uiprivProgrammerErrorPrefix "libui programmer error"
// TODO add debugging advice?
#define uiprivProgrammerErrorAdvice "This likely means you are using libui incorrectly. Check your source code and try again. If you have received this warning in error, contact the libui authors."

#ifdef __cplusplus
}
#endif
