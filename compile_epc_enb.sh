cd /home/jwang/work/srsRAN_4G/build
rm -rf *
cmake ../ > cmake_tmp.log 2>&1
make -j $(nproc) > make_tmp.log 2>&1
cd ../
