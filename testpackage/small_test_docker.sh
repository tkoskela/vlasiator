cores=4
threads=1
export OMP_NUM_THREADS=$t

#command for running stuff
run_command="mpirun -n $cores --allow-run-as-root"
small_run_command="mpirun -n 1 --allow-run-as-root"
run_command_tools="mpirun -n 1 --allow-run-as-root"

#If 1, the reference vlsv files are generated
# if 0 then we check the v1
create_verification_files=0

#folder for all reference data 
reference_dir="/home/vlasiator/testpackage/"
#compare agains which revision. This can be a proper version string, or "current", which should be a symlink to the
#proper most recent one
reference_revision="current"

# Define test
source small_test_definitions.sh
wait

run_tests=( 1 2 3 )

# Run tests
source run_tests.sh
