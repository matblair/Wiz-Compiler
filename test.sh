for OUTPUT in $(ls testprograms/)                                                                    
do
	echo "Testing "$OUTPUT
	./wiz -p  testprograms/$OUTPUT >testoutput/$OUTPUT.out
	#cat testoutput/$OUTPUT.out >> results.txt
done
