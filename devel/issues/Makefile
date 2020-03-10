all : issue_124 issue_239 issue_34 issue_379 issue_45 issue_47 issue_93

# default is just call the file
% : %.py
	python3 $<

issue_379: issue_379.py
	python3 -m memory_profiler $<
