#
# This is the super Makefile for the ARC_API libraries.
#

# This directory is not required. It's only a convenience for copying
# the libraries into Owl before it's bundled.
OWL_DIR			=	$(HOME)/Documents/JAVA/Owl/3.0


# REQUIRED - Change this to match the location of the API source.
RELEASE_DIR		=	$(PWD)/Release


X64_COPY		=	cp -f x64/*.so $(RELEASE_DIR)/x64
X32_COPY		=	cp -f x32/*.so $(RELEASE_DIR)/x32


X64_OWL_COPY		=	cp -f x64/*.so $(OWL_DIR)/x64/API
X32_OWL_COPY		=	cp -f x32/*.so $(OWL_DIR)/x32/API


all: m64 m32

m64:
	@echo
	@echo ------------------------------------------------------------------------------
	@echo  Bulding CArcDevice x64
	@echo ------------------------------------------------------------------------------
	cd CArcDevice/linux; $(MAKE) m64; $(X64_COPY); \
	if test -d $(OWL_DIR ); then $(X64_OWL_COPY); fi

	@echo
	@echo ------------------------------------------------------------------------------
	@echo  Building CDeinterlace x64
	@echo ------------------------------------------------------------------------------
	cd CDeinterlace/linux; $(MAKE) m64; $(X64_COPY); \
	if test -d $(OWL_DIR ); then $(X64_OWL_COPY); fi

	@echo
	@echo ------------------------------------------------------------------------------
	@echo  Building CDisplay x64
	@echo ------------------------------------------------------------------------------
	cd CDisplay/linux; $(MAKE) m64; $(X64_COPY); \
	if test -d $(OWL_DIR ); then $(X64_OWL_COPY); fi

	@echo
	@echo ------------------------------------------------------------------------------
	@echo  Building CFitsFile x64
	@echo ------------------------------------------------------------------------------
	cd CFitsFile/linux; $(MAKE) m64; $(X64_COPY); \
	if test -d $(OWL_DIR ); then $(X64_OWL_COPY); fi

	@echo
	@echo ------------------------------------------------------------------------------
	@echo  Building CImage x64
	@echo ------------------------------------------------------------------------------
	cd CImage/linux; $(MAKE) m64; $(X64_COPY); \
	if test -d $(OWL_DIR ); then $(X64_OWL_COPY); fi

	@echo
	@echo ------------------------------------------------------------------------------
	@echo  Building CTiffFile x64
	@echo ------------------------------------------------------------------------------
	cd CTiffFile/linux; $(MAKE) m64; $(X64_COPY); \
	if test -d $(OWL_DIR ); then $(X64_OWL_COPY); fi

	@echo
	@echo ------------------------------------------------------------------------------
	@echo  Building CArcAPIClient x64
	@echo ------------------------------------------------------------------------------
	cd CArcAPIClient/linux; $(MAKE) m64; $(X64_COPY); \
	if test -d $(OWL_DIR ); then $(X64_OWL_COPY); fi

	@echo
	@echo ------------------------------------------------------------------------------
	@echo  Bulding CameraAPI x64
	@echo ------------------------------------------------------------------------------
	cd CameraAPI/linux; $(MAKE) m64; $(X64_COPY); \
	if test -d $(OWL_DIR ); then $(X64_OWL_COPY); fi

	@echo
	@echo ------------------------------------------------------------------------------
	@echo  Bulding ArcCameraAPI x64
	@echo ------------------------------------------------------------------------------
	cd ArcCameraAPI/linux; $(MAKE) m64; $(X64_COPY)

	@echo
	@echo ------------------------------------------------------------------------------
	@echo  Building ArcAPIService x64
	@echo ------------------------------------------------------------------------------
	cd ArcAPIService/linux; $(MAKE) m64; cp -f x64/ArcAPIService $(RELEASE_DIR)/x64

m32:
	@echo  
	@echo ------------------------------------------------------------------------------
	@echo  Bulding CArcDevice x32 
	@echo ------------------------------------------------------------------------------
	cd CArcDevice/linux; $(MAKE) m32; $(X32_COPY); \
	if test -d $(OWL_DIR ); then $(X32_OWL_COPY); fi

	@echo
	@echo ------------------------------------------------------------------------------
	@echo  Building CDeinterlace x32
	@echo ------------------------------------------------------------------------------
	cd CDeinterlace/linux; $(MAKE) m32; $(X32_COPY); \
	if test -d $(OWL_DIR ); then $(X32_OWL_COPY); fi

	@echo
	@echo ------------------------------------------------------------------------------
	@echo  Building CDisplay x32
	@echo ------------------------------------------------------------------------------
	cd CDisplay/linux; $(MAKE) m32; $(X32_COPY); \
	if test -d $(OWL_DIR ); then $(X32_OWL_COPY); fi

	@echo
	@echo ------------------------------------------------------------------------------
	@echo  Building CFitsFile x32
	@echo ------------------------------------------------------------------------------
	cd CFitsFile/linux; $(MAKE) m32; $(X32_COPY); \
	if test -d $(OWL_DIR ); then $(X32_OWL_COPY); fi

	@echo
	@echo ------------------------------------------------------------------------------
	@echo  Building CImage x32
	@echo ------------------------------------------------------------------------------
	cd CImage/linux; $(MAKE) m32; $(X32_COPY); \
	if test -d $(OWL_DIR ); then $(X32_OWL_COPY); fi

	@echo
	@echo ------------------------------------------------------------------------------
	@echo  Building CTiffFile x32
	@echo ------------------------------------------------------------------------------
	cd CTiffFile/linux; $(MAKE) m32; $(X32_COPY); \
	if test -d $(OWL_DIR ); then $(X32_OWL_COPY); fi

	@echo
	@echo ------------------------------------------------------------------------------
	@echo  Building CArcAPIClient x32
	@echo ------------------------------------------------------------------------------
	cd CArcAPIClient/linux; $(MAKE) m32; $(X32_COPY); \
	if test -d $(OWL_DIR ); then $(X32_OWL_COPY); fi

	@echo  
	@echo ------------------------------------------------------------------------------
	@echo  Bulding CameraAPI x32 
	@echo ------------------------------------------------------------------------------
	cd CameraAPI/linux; $(MAKE) m32; $(X32_COPY); \
	if test -d $(OWL_DIR ); then $(X32_OWL_COPY); fi 

	@echo  
	@echo ------------------------------------------------------------------------------
	@echo  Bulding ArcCameraAPI x32 
	@echo ------------------------------------------------------------------------------
	cd ArcCameraAPI/linux; $(MAKE) m32; $(X32_COPY)

	@echo
	@echo ------------------------------------------------------------------------------
	@echo  Building ArcAPIService x32
	@echo ------------------------------------------------------------------------------
	cd ArcAPIService/linux; $(MAKE) m32; cp -f x32/ArcAPIService $(RELEASE_DIR)/x32 

clean: clean_m64 clean_m32

clean_m64:
	cd CArcDevice/linux; $(MAKE) clean_m64
	cd CDeinterlace/linux; $(MAKE) clean_m64
	cd CDisplay/linux; $(MAKE) clean_m64
	cd CFitsFile/linux; $(MAKE) clean_m64
	cd CImage/linux; $(MAKE) clean_m64
	cd CArcAPIClient/linux; $(MAKE) clean_m64
	cd CameraAPI/linux; $(MAKE) clean_m64
	cd ArcCameraAPI/linux; $(MAKE) clean_m64
	cd ArcAPIService/linux; $(MAKE) clean_m64

clean_m32:
	cd CArcDevice/linux; $(MAKE) clean_m32
	cd CDeinterlace/linux; $(MAKE) clean_m32
	cd CDisplay/linux; $(MAKE) clean_m32
	cd CFitsFile/linux; $(MAKE) clean_m32
	cd CImage/linux; $(MAKE) clean_m32
	cd CArcAPIClient/linux; $(MAKE) clean_m32
	cd CameraAPI/linux; $(MAKE) clean_m32
	cd ArcCameraAPI/linux; $(MAKE) clean_m32
	cd ArcAPIService/linux; $(MAKE) clean_m32


