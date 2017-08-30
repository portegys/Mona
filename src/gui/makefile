# GUI makefile

OBJECTS = Font.o GLee.o GUIAlphaElement.o GUIButton.o GUICheckBox.o \
  GUIClippedRectangle.o GUIComboBox.o GUIEvent.o GUIFont.o GUIFontManager.o \
  GUIFrame.o GUILabel.o GUIPanel.o GUIRadioButton.o GUIRectangle.o \
  GUISeparator.o GUISlider.o GUITabbedPanel.o GUITexCoordDescriptor.o \
  GUIText.o GUITextBox.o GenUtils.o TextureUtils.o TimeUtils.o XMLUtils.o

LIB = ../../lib/libsxmlgui.a

CC = cc

CCFLAGS = -O3

all : $(LIB)

$(LIB) : $(OBJECTS)
	@(cd glpng; make)
	mkdir -p ../../lib
	ar -cr -o $(LIB) $(OBJECTS)

Font.o : Font.h GenUtils.h Font.cpp
	$(CC) $(CCFLAGS) -c Font.cpp

GLee.o : GLee.h GLee.cpp
	$(CC) $(CCFLAGS) -c GLee.cpp

GUIAlphaElement.o : EasyGL.h GUIAlphaElement.cpp
	$(CC) $(CCFLAGS) -c GUIAlphaElement.cpp

GUIButton.o : EasyGL.h GUIButton.cpp
	$(CC) $(CCFLAGS) -c GUIButton.cpp

GUICheckBox.o : EasyGL.h GUICheckBox.cpp
	$(CC) $(CCFLAGS) -c GUICheckBox.cpp

GUIClippedRectangle.o : EasyGL.h GUIClippedRectangle.cpp
	$(CC) $(CCFLAGS) -c GUIClippedRectangle.cpp

GUIComboBox.o : EasyGL.h GUIComboBox.cpp
	$(CC) $(CCFLAGS) -c GUIComboBox.cpp

GUIEvent.o : EasyGL.h GUIEvent.cpp
	$(CC) $(CCFLAGS) -c GUIEvent.cpp

GUIFont.o : EasyGL.h GUIFont.cpp
	$(CC) $(CCFLAGS) -c GUIFont.cpp

GUIFontManager.o : EasyGL.h GUIFontManager.cpp
	$(CC) $(CCFLAGS) -c GUIFontManager.cpp

GUIFrame.o : EasyGL.h GUIFrame.cpp
	$(CC) $(CCFLAGS) -c GUIFrame.cpp

GUILabel.o : EasyGL.h GUILabel.cpp
	$(CC) $(CCFLAGS) -c GUILabel.cpp

GUIPanel.o : EasyGL.h GUIPanel.cpp
	$(CC) $(CCFLAGS) -c GUIPanel.cpp

GUIRadioButton.o : EasyGL.h GUIRadioButton.cpp
	$(CC) $(CCFLAGS) -c GUIRadioButton.cpp

GUIRectangle.o : EasyGL.h GUIRectangle.cpp
	$(CC) $(CCFLAGS) -c GUIRectangle.cpp

GUISeparator.o : EasyGL.h GUISeparator.cpp
	$(CC) $(CCFLAGS) -c GUISeparator.cpp

GUISlider.o : EasyGL.h GUISlider.cpp
	$(CC) $(CCFLAGS) -c GUISlider.cpp

GUITabbedPanel.o : EasyGL.h GUITabbedPanel.cpp
	$(CC) $(CCFLAGS) -c GUITabbedPanel.cpp

GUITexCoordDescriptor.o : EasyGL.h GUITexCoordDescriptor.cpp
	$(CC) $(CCFLAGS) -c GUITexCoordDescriptor.cpp

GUIText.o : EasyGL.h GUIText.cpp
	$(CC) $(CCFLAGS) -c GUIText.cpp

GUITextBox.o : EasyGL.h GUITextBox.cpp
	$(CC) $(CCFLAGS) -c GUITextBox.cpp

GenUtils.o : GenUtils.h GenUtils.cpp
	$(CC) $(CCFLAGS) -c GenUtils.cpp

TextureUtils.o : TextureUtils.h TextureUtils.cpp
	$(CC) $(CCFLAGS) -c TextureUtils.cpp

TimeUtils.o : GenUtils.h TimeUtils.h TimeUtils.cpp
	$(CC) $(CCFLAGS) -c TimeUtils.cpp

XMLUtils.o : GenUtils.h XMLUtils.h XMLUtils.cpp
	$(CC) $(CCFLAGS) -c XMLUtils.cpp

clean:
	@(cd glpng; make clean)
	/bin/rm -f *.o

