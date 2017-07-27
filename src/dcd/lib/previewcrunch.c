#include <Python.h>
static PyObject *
previewcrunch_downconvert(self, args)
    PyObject *self;
    PyObject *args;
{
	PyObject * result;
	unsigned char *buffer;
	unsigned char *displayBuffer;
	int ok,size,BytesPerPixel,x,b;
	unsigned int maxGoodData;
	double temp;
	double bright,contrast,gamma,val,offset,scale,dat;
	ok = PyArg_ParseTuple(args, "s#dddid", &buffer, &size, &bright,&contrast,&gamma,&BytesPerPixel,&temp);
	maxGoodData=(unsigned int) temp;
	if (!ok) {
		return NULL;
	}
	displayBuffer=malloc((3*(size/BytesPerPixel))*sizeof(unsigned  char));
	offset=pow(pow(256.0,BytesPerPixel),1.0-bright)-1.0;
	scale=pow(pow(256.0,BytesPerPixel+1),contrast)-1.0;
	for (x=0;x<size/BytesPerPixel;x++) {
		dat=0;
		for (b=0;b<BytesPerPixel;b++) {
			dat=(unsigned long) (dat+ buffer[x*BytesPerPixel+b]*pow(256.0,b));
		}
		if (dat<=maxGoodData) {
			val=128+scale*(dat-offset)/(pow(256.0,BytesPerPixel)-1.0);
			if (val<0.0) {
				val=0.0;
			}
			val=pow(val/255.0,1.0/(gamma))*255.0;
			if (val>255.0) {
				val=255.0; 
			}
			val=round(val*(254.0/255.0));
			displayBuffer[3*x]=(int) val;
			displayBuffer[3*x+1]=(int) val;
			displayBuffer[3*x+2]=(int) val;
			
		}	
		else {
			displayBuffer[3*x]=255;
			displayBuffer[3*x+1]=0;
			displayBuffer[3*x+2]=0;
		}	
	}
	result=Py_BuildValue("s#", displayBuffer , 3*(size/BytesPerPixel)  );
	free(displayBuffer);
	return result;
}


static PyObject *
previewcrunch_reorder(self, args)
    PyObject *self;
    PyObject *args;
{
	PyObject * result;
	unsigned char *converted;
	unsigned char *pbuf;
	int ok,size;
	unsigned int pos,i,offs;
	ok = PyArg_ParseTuple(args, "s#", &converted, &size);
	if (!ok) {
		return NULL;
	}
	pbuf=malloc(size*sizeof(unsigned  char));
	for (i=0;i<size/3;i++) {
		if (i%2==0) {
			offs=i/2;
		}
		if ((i%2)<1){
			pos=3*(i-offs);
			pbuf[pos]=converted[3*i];
			pbuf[pos+1]=converted[3*i+1];
			pbuf[pos+2]=converted[3*i+2];
		}
		else {
			pos=3*(size/3-(i-offs)-1);
			pbuf[pos]=converted[3*i];
			pbuf[pos+1]=converted[3*i+1];
			pbuf[pos+2]=converted[3*i+2];
		}
	}
	result=Py_BuildValue("s#", pbuf , size  );
	return result;
}



static PyMethodDef PreviewCrunchMethods[] = {
    {"downconvert",  previewcrunch_downconvert, METH_VARARGS,
     "Downconvert the preview."},
    {"reorder",  previewcrunch_reorder, METH_VARARGS,
     "Put Byteoder=2 Data in correct order"},     
    {NULL, NULL, 0, NULL}        /* Sentinel */
};

PyMODINIT_FUNC
initpreviewcrunch(void)
{
    (void) Py_InitModule("previewcrunch", PreviewCrunchMethods);
}
