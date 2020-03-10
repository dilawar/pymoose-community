all : 93

# default is just call the file
% : issue_%.py
	python3 $< | tee -a $@.log

# This issue requires memory profiling. 
# # WONTFIX.
379: issue_379.py
	python3 -m pip install memprof --user
	# Witch threshold of 1 kB
	python3 -m memprof --threshold 1024 --plot $< | tee -a $@.log
