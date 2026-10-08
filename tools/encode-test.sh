#!/bin/sh
# Encode on the VPU from each input format and size; PSNR against the source.
# Run in a scratch directory on the board, with amvx loaded.
for t in nv12:1280x720 yuv420p:1280x720 nv12:1920x1080 yuv420p:1920x1080 nv21:640x480; do
	pf=${t%%:*}; sz=${t##*:}
	ffmpeg -hide_banner -loglevel error -y -f lavfi -i testsrc2=size=$sz:rate=30 -frames:v 30 \
	    -pix_fmt $pf -c:v h264_v4l2m2m -b:v 8M e.h264 > e.err 2>&1 || { echo "$t: encode failed: $(head -1 e.err)"; continue; }
	p=$(ffmpeg -hide_banner -r 30 -i e.h264 -f lavfi -i testsrc2=size=$sz:rate=30 -filter_complex \
	    "[0:v]setpts=N[a];[1:v]trim=end_frame=30,setpts=N,format=yuv420p[b];[a][b]psnr" -f null - 2>&1 | grep -o "PSNR.*")
	echo "$t: $p"
done
sync
