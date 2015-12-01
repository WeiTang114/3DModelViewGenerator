import subprocess as sp
from os import listdir
from os.path import isfile, join

MODELS_DIR = '/Users/weitang114/Dev/Sketch3d/dataset/SHREC13_SBR_TARGET_MODELS/models'

def main():
	files = [ f for f in listdir(MODELS_DIR) if isfile(join(MODELS_DIR,f)) ]
	ids = [ f.split('.')[0][1:] for f in files ]


	CMD = './view_generator %s'

	for idd in ids:
		if not idd.isdigit(): continue
		print idd
		cmd = CMD % idd
		print 'cmd:', cmd
		sp.check_output(cmd, shell=True)


if __name__=='__main__':
	main()

