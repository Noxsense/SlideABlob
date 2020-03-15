
# convert -size 800x800 xc:white ~/emulated.0/white.png
# convert -size 800x800 xc:transparent ~/emulated.0/transparent.png

BMP="${0%%.sh}.bmp"

SIZE_FIELD=32

convert \
	\( -size ${SIZE_FIELD}x${SIZE_FIELD} xc:'#000000' \) \
	\( -size ${SIZE_FIELD}x${SIZE_FIELD} xc:'#aaaaaa' \) \
	\( -size ${SIZE_FIELD}x${SIZE_FIELD} xc:'#ff0000' \) \
	\( -size ${SIZE_FIELD}x${SIZE_FIELD} xc:'#00ff00' \) \
	\( -size ${SIZE_FIELD}x${SIZE_FIELD} xc:'#0000ff' \) \
	\( -size ${SIZE_FIELD}x${SIZE_FIELD} xc:'#ffff00' \) \
	\( -size ${SIZE_FIELD}x${SIZE_FIELD} xc:'#00ffff' \) \
	\( -size ${SIZE_FIELD}x${SIZE_FIELD} xc:'#ff00ff' \) \
	+append \
	"${BMP}" && \
	echo "Updated new: '${BMP}'"
