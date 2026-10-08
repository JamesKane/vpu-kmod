#!/bin/sh
# Decode each codec on the VPU; compare frame MD5s with software decoding.
# Run in a scratch directory on the board, with amvx loaded.

mk() { # name size encoder-args
	[ -f $1.mkv ] || ffmpeg -hide_banner -loglevel error -y -f lavfi \
	    -i testsrc2=size=$2:rate=30 -frames:v 120 -pix_fmt yuv420p $3 $1.mkv
}
mk h264-1080p 1920x1080 "-c:v libx264 -bf 2"
mk hevc-1080p 1920x1080 "-c:v libx265 -x265-params log-level=error"
mk vp9-720p 1280x720 "-c:v libvpx-vp9 -deadline realtime -cpu-used 8"
for t in h264-1080p:h264_v4l2m2m hevc-1080p:hevc_v4l2m2m vp9-720p:vp9_v4l2m2m; do
	f=${t%%:*}; d=${t##*:}
	ffmpeg -hide_banner -loglevel error -i $f.mkv -pix_fmt yuv420p -f framemd5 - 2>/dev/null | grep -v "^#" > sw-$f.txt
	t0=$(date +%s.%N)
	ffmpeg -hide_banner -loglevel error -c:v $d -i $f.mkv -pix_fmt yuv420p -f framemd5 - 2>hw-$f.err | grep -v "^#" > hw-$f.txt
	t1=$(date +%s.%N)
	if cmp -s sw-$f.txt hw-$f.txt; then r=MATCH; else r="DIFF ($(wc -l < hw-$f.txt) frames, $(head -1 hw-$f.err))"; fi
	echo "$f $d: $r, $(echo "$t1 - $t0" | bc) s"
done
sync
