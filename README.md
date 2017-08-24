singularityUApplication

git clone https://github.com/kazukitakata/singularityU2017.git

'
$ cmake .
$ make all -j${number_of_cpu_core}
'

'
$ ./bin/face_detection video {address of file} -j${number_of_cpu_core}
$ ./bin/face_detection camera -j${number_of_cpu_core}
'
