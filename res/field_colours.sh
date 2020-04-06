
# convert -size 800x800 xc:white ~/emulated.0/white.png
# convert -size 800x800 xc:transparent ~/emulated.0/transparent.png

BMP="${0%%.sh}.bmp"

SIZE_FIELD=32
FONT_SIZE=25

text1="fill white text 0,0 '1' "
text2="fill white text 0,0 '2' "
text3="fill white text 0,0 '3' "
text4="fill white text 0,0 '4' "
text5="fill white text 0,0 '5' "
text6="fill white text 0,0 '6' "
text7="fill white text 0,0 '7' "

ALPHA='#eeeeee'

convert \
	-pointsize $FONT_SIZE \
	-gravity center \
	\( -size ${SIZE_FIELD}x${SIZE_FIELD} xc:${ALPHA} \) \
	\( -size ${SIZE_FIELD}x${SIZE_FIELD} xc:'#aaaaaa' -draw "$text1" \) \
	\( -size ${SIZE_FIELD}x${SIZE_FIELD} xc:'#ff0000' -draw "$text2" \) \
	\( -size ${SIZE_FIELD}x${SIZE_FIELD} xc:'#00ff00' -draw "$text3" \) \
	\( -size ${SIZE_FIELD}x${SIZE_FIELD} xc:'#0000ff' -draw "$text4" \) \
	\( -size ${SIZE_FIELD}x${SIZE_FIELD} xc:'#ffff00' -draw "$text5" \) \
	\( -size ${SIZE_FIELD}x${SIZE_FIELD} xc:'#00ffff' -draw "$text6" \) \
	\( -size ${SIZE_FIELD}x${SIZE_FIELD} xc:'#000000' -draw "$text7" \) \
	+append \
	"${BMP}" && \
	echo "Updated new: '${BMP}'"

cd ~/Projects/SlideALama-clone

[[ ( -e output ) ]] && cp -uv ${BMP} output/res/
