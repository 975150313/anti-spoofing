1���ɼ���������ݼ���ִ��
    python data_preprocess.py ��ori_dirĿ¼�е����ݼ���Ϊѵ�����Ͳ��Լ�������ŵ�new_dir���ļ����У�
2����tools�ļ����е�im2rec.py�ļ�������new_dir�ļ����У�Ȼ������ִ�У�
	python im2rec.py --recursive --exts=.png --list train train
	python im2rec.py --recursive --exts=.png --list test test
	python im2rec.py ./test.lst test
	python im2rec.py ./train.lst train
    ע������windows�汾��mxnet���⣬ִ�г���󣬽��̲������˳�����Ҫ�ֶ�����python.exe���̣�
        �������ɵ�train.rec��test.rec���Ǵ���������ֱ������ѵ�������ݼ���