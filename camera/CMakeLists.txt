
SET(SRCS 
avilib.c
avilib.h
button.h
camera.c
camera.h
color.c
color.h
gui.c
gui.h
huffman.h
utils.c
utils.h
uvc_compat.h
uvcvideo.h
v4l2uvc.c
v4l2uvc.h
)
ADD_LIBRARY( camera STATIC ${SRCS} )
