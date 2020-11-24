#installed opencv via 
#https://docs.opencv.org/4.5.0/d7/d9f/tutorial_linux_install.html
#Then did instructions here 
# https://github.com/cggos/dip_cvqt/issues/1
#Find the folder containing the shared library libopencv_core.so.3.2 using the following command line.
#sudo find / -name libopencv_calib3d.so.4.5
#Then I got the result: /usr/local/lib/libopencv_core.so.3.2.
#2. Create a file called /etc/ld.so.conf.d/opencv.conf and write to it the path to the folder where the binary is stored.For example, I wrote /usr/local/lib/ to my opencv.conf file.
#3. Run the command line as follows.

g++ -std=c++11 -O3 -I /usr/local/include/opencv4/ align_and_merge.cc -L /usr/local/lib/ -lopencv_stitching -lopencv_superres -lopencv_videostab -lopencv_photo -lopencv_aruco -lopencv_bgsegm -lopencv_bioinspired -lopencv_ccalib -lopencv_dpm -lopencv_face -lopencv_fuzzy -lopencv_img_hash -lopencv_line_descriptor -lopencv_optflow -lopencv_reg -lopencv_rgbd -lopencv_saliency -lopencv_stereo -lopencv_structured_light -lopencv_phase_unwrapping -lopencv_surface_matching -lopencv_tracking -lopencv_datasets -lopencv_text -lopencv_dnn -lopencv_plot -lopencv_xfeatures2d -lopencv_shape -lopencv_video -lopencv_ml -lopencv_ximgproc -lopencv_calib3d -lopencv_features2d -lopencv_highgui -lopencv_videoio -lopencv_flann -lopencv_xobjdetect -lopencv_imgcodecs -lopencv_objdetect -lopencv_xphoto -lopencv_imgproc -lopencv_core -o align_and_merge


