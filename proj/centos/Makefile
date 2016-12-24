CC=LD_LIBRARY_PATH=/usr/local/lib64 g++ `pkg-config --cflags --libs glib-2.0` -I/usr/local/include/SDL2/ -I/usr/local/include/ -I/usr/include/ -I/usr/include/cairo/ -I/usr/include/pango-1.0/ -I/usr/include/gtk-3.0/ -I/usr/lib64/gtk-3.0/include/ -I/usr/include/gtk-2.0/gdk/ -I/usr/include/glib-2.0/ -I/usr/include/glib-2.0/include/ -I/usr/include/gdk-pixbuf-2.0/ -I/usr/include/atk-1.0/
CFLAGS=-c -Wall -std=c++0x
LDFLAGS=-L/usr/lib64 -lGL -lSDL2 -lGLEW -lpng -ljpeg -lm -lglib-2.0 -lgtk-3 -lgobject-2.0 -L/usr/local/lib -Wl,-R/usr/local/lib/ -lassimp
SOURCES=../../source/app/appmain.cpp\
../../source/app/render.cpp\
../../source/app/segui.cpp\
../../source/app/sesim.cpp\
../../source/app/seviewport.cpp\
../../source/app/undo.cpp\
../../source/debug.cpp\
../../source/platform.cpp\
../../source/utils.cpp\
../../source/sys/unicode.cpp\
../../source/sys/utf8.cpp\
../../source/sys/mempool.cpp\
../../source/tool/rendersprite.cpp\
../../source/tool/compilebl.cpp\
../../source/tool/rendertopo.cpp\
../../source/window.cpp\
../../source/ustring.cpp\
../../source/texture.cpp\
../../source/math/math3d.cpp\
../../source/math/camera.cpp\
../../source/math/matrix.cpp\
../../source/math/quaternion.cpp\
../../source/math/vec3f.cpp\
../../source/math/vec4f.cpp\
../../source/math/plane3f.cpp\
../../source/math/plane.cpp\
../../source/math/brush.cpp\
../../source/math/brushside.cpp\
../../source/math/edge.cpp\
../../source/math/frustum.cpp\
../../source/math/hmapmath.cpp\
../../source/math/line.cpp\
../../source/math/polygon.cpp\
../../source/render/shader.cpp\
../../source/render/billboard.cpp\
../../source/render/foliage.cpp\
../../source/render/heightmap.cpp\
../../source/render/model2.cpp\
../../source/render/ms3d.cpp\
../../source/render/particle.cpp\
../../source/render/projectile.cpp\
../../source/render/screenshot.cpp\
../../source/render/shadow.cpp\
../../source/render/sortb.cpp\
../../source/render/sprite.cpp\
../../source/render/vertexarray.cpp\
../../source/render/water.cpp\
../../source/save/save.cpp\
../../source/save/compilemap.cpp\
../../source/save/edmap.cpp\
../../source/save/modelholder.cpp\
../../source/save/saveedm.cpp\
../../source/save/savemap.cpp\
../../source/save/savesprite.cpp\
../../source/sim/door.cpp\
../../source/sim/entity.cpp\
../../source/sim/entitytype.cpp\
../../source/sim/explocrater.cpp\
../../source/sim/map.cpp\
../../source/sim/selection.cpp\
../../source/sim/sim.cpp\
../../source/sim/tile.cpp\
../../source/phys/collider.cpp\
../../source/gui/cursor.cpp\
../../source/gui/inevent.cpp\
../../source/gui/gui.cpp\
../../source/gui/widget.cpp\
../../source/gui/icon.cpp\
../../source/gui/font.cpp\
../../source/gui/richtext.cpp\
../../source/gui/viewlayer.cpp\
../../source/gui/draw2d.cpp\
../../source/gui/widgets/winw.cpp\
../../source/gui/widgets/image.cpp\
../../source/gui/widgets/button.cpp\
../../source/gui/widgets/editbox.cpp\
../../source/gui/widgets/text.cpp\
../../source/gui/widgets/droplist.cpp\
../../source/gui/widgets/barbutton.cpp\
../../source/gui/widgets/hscrollbar.cpp\
../../source/gui/widgets/insdraw.cpp\
../../source/gui/widgets/link.cpp\
../../source/gui/widgets/viewportw.cpp\
../../source/gui/widgets/listbox.cpp\
../../source/gui/widgets/pane.cpp\
../../source/gui/widgets/textarea.cpp\
../../source/gui/widgets/textblock.cpp\
../../source/gui/widgets/frame.cpp\
../../source/gui/widgets/checkbox.cpp\
../../source/gui/widgets/vscrollbar.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=../../testfolder/pl

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf *o $(EXECUTABLE)
	rm -rf $(OBJECTS) 
