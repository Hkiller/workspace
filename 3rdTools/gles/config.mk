product:=opengl
$(product).type:=virtual

$(product).mingw.product.c.includes:=3rdTools/gles/mingw/include
$(product).mingw.product.c.libraries:=opengl32

$(product).ios.product.c.frameworks:=OpenGL OpenGLES

$(product).mac.product.c.frameworks:=OpenGL

$(product).android.product.c.libraries+=GLESv2

$(eval $(call product-def,$(product)))
