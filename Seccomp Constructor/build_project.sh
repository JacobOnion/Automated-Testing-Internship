set -e

./premake5 gmake
cd build
make clean
make

cd ..

#echo -e "\n\n RUNNING PRISTINE CODE\n\n"

#build/RunPristine

#echo -e "\n\n\n\n\n\n\n RUNNING SUBMISSION\n\n"

#build/RunSubmission