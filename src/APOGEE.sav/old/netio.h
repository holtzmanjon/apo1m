extern unsigned short PASCAL NetRead1(unsigned short reg);
extern unsigned long PASCAL NetReadN(unsigned short reg, unsigned long cnt, unsigned short *pData);
extern void PASCAL NetWrite1(unsigned short reg, unsigned short val);
extern BOOL PASCAL NetInit(char *szServer, UINT uiPort);
extern void PASCAL NetFini(void);
extern void PASCAL NetPriority(void);
