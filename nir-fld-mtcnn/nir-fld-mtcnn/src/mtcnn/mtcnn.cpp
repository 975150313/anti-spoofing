//Created by Kai Zuo
#include <mtcnn/mtcnn.h>

const float pnet_stride = 2;
const float pnet_cell_size = 12;
const int pnet_max_detect_num = 5000;
//mean & std
const float mean_val = 127.5f;
const float std_val = 0.0078125f;
//minibatch size
const int step_size = 128;


bool CompareBBox(const FaceInfo & a, const FaceInfo & b) {
	return a.bbox.score > b.bbox.score;
}

Rect FaceInfo2Rect(FaceInfo& faceInfo)
{
	int x = (int)faceInfo.bbox.xmin;
	int y = (int)faceInfo.bbox.ymin;
	int w = (int)(faceInfo.bbox.xmax - faceInfo.bbox.xmin + 1);
	int h = (int)(faceInfo.bbox.ymax - faceInfo.bbox.ymin + 1);
	return cv::Rect(x, y, w, h);
}

float faceInfoArea(FaceInfo& faceInfo)
{
	float w = (faceInfo.bbox.xmax - faceInfo.bbox.xmin + 1);
	float h = (faceInfo.bbox.ymax - faceInfo.bbox.ymin + 1);
	return w * h;
}

FaceSize getFaceSize(FaceInfo& faceInfo)
{
	float w = (faceInfo.bbox.xmax - faceInfo.bbox.xmin + 1);
	float h = (faceInfo.bbox.ymax - faceInfo.bbox.ymin + 1);
	FaceSize faceSize;
	faceSize.height = h;
	faceSize.width = w;
	return faceSize;
}

void cropFace4Flow(Mat& img, FaceInfo& faceInfo, Rect& cropInfo, int padding)
{
	if (padding < 0)
		padding = 0;

	float xmin = faceInfo.bbox.xmin;
	float xmax = faceInfo.bbox.xmax;
	float ymin = faceInfo.bbox.ymin;
	float ymax = faceInfo.bbox.ymax;

	float img_xmax = img.cols;
	float img_ymax = img.rows;

	int clip_xmin = xmin < padding ? 0 : xmin - padding;
	int clip_ymin = ymin < padding ? 0 : ymin - padding;
	int clip_xmax = (img_xmax - xmax) < padding ? img_xmax : xmax + padding;
	int clip_ymax = (img_ymax - ymax) < padding ? img_ymax : ymax + padding;
	int clip_w = clip_xmax - clip_xmin;
	int clip_h = clip_ymax - clip_ymin;

	cropInfo = Rect(clip_xmin, clip_ymin, clip_w, clip_h);
}

//修正人脸坐标
void amendFaceAxis(Mat org_img, vector<FaceInfo>& facesInfo, int crop_width, int crop_height) {
	int height = org_img.rows;
	int width = org_img.cols;

	int offset_x = (width - crop_width) / 2;
	int offset_y = (height - crop_height) / 2;

	for (int i = 0; i < facesInfo.size(); i++) {
		int offset_x = (width - crop_width) / 2;
		facesInfo[i].bbox.xmin += offset_x;
		facesInfo[i].bbox.ymin += offset_y;
		facesInfo[i].bbox.xmax += offset_x;
		facesInfo[i].bbox.ymax += offset_y;
		for (int j = 0; j < 10; j++) {
			if (j % 2 == 0) {
				facesInfo[i].landmark[j] += offset_x;
			}
			else {
				facesInfo[i].landmark[j] += offset_y;
			}
		}
		
	}
}

//修正人脸坐标
void amendFaceAxis(Mat org_img, vector<FaceInfo_ncnn>& facesInfo, int crop_width, int crop_height) {
	int height = org_img.rows;
	int width = org_img.cols;

	int offset_x = (width - crop_width) / 2;
	int offset_y = (height - crop_height) / 2;

	for (int i = 0; i < facesInfo.size(); i++) {
		int offset_x = (width - crop_width) / 2;
		facesInfo[i].x[0] += offset_x;
		facesInfo[i].y[0] += offset_y;
		facesInfo[i].x[1] += offset_x;
		facesInfo[i].y[1] += offset_y;
	}
}

FaceInfo drawRectangle(Mat& img, vector<FaceInfo>& v)
{
	FaceInfo maxFaceInfo = FaceInfo{};
	if (!v.empty())
	{
		for (int i = 0; i < v.size(); i++) {
			FaceInfo faceInfo = v[i];
			int x = (int)faceInfo.bbox.xmin;
			int y = (int)faceInfo.bbox.ymin;
			int w = (int)(faceInfo.bbox.xmax - faceInfo.bbox.xmin + 1);
			int h = (int)(faceInfo.bbox.ymax - faceInfo.bbox.ymin + 1);
			stringstream str_x, str_y, str_h, str_w;
			str_x << x;
			str_y << y;
			str_h << h;
			str_w << w;
			putText(img, "x:" + str_x.str() + ",y:" + str_y.str() + ",h:" + str_h.str() + ",w:" + str_w.str(), cv::Point(x, y), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(0, 0, 255), 1);
			cv::rectangle(img, cv::Rect(x, y, w, h), cv::Scalar(0, 255, 0), 1);

			int area = w * h;

			if (0 == i)
			{
				maxFaceInfo = faceInfo;
			}
			else
			{
				int max_w = (int)(maxFaceInfo.bbox.xmax - maxFaceInfo.bbox.xmin + 1);
				int max_h = (int)(maxFaceInfo.bbox.ymax - maxFaceInfo.bbox.ymin + 1);
				int max_area = max_w * max_h;
				if (area > max_area)
				{
					maxFaceInfo = faceInfo;
				}
			}
		}
		putText(img, "maxFace", Point(maxFaceInfo.bbox.xmin, maxFaceInfo.bbox.ymin - 20), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(0, 255, 0), 1);
	}
	
	return maxFaceInfo;
} 

FaceInfo_ncnn getMaxFaceInfo(vector<FaceInfo_ncnn>& v)
{
	FaceInfo_ncnn maxFaceInfo = FaceInfo_ncnn{};
	if (!v.empty())
	{
		for (int i = 0; i < v.size(); i++) {
			FaceInfo_ncnn faceInfo = v[i];
			int x = (int)faceInfo.x[0];
			int y = (int)faceInfo.y[0];
			int w = (int)(faceInfo.x[1] - faceInfo.x[0] + 1);
			int h = (int)(faceInfo.y[1] - faceInfo.y[0] + 1);
			int area = w * h;

			if (0 == i)
			{
				maxFaceInfo = faceInfo;
			}
			else
			{
				int max_w = (int)(maxFaceInfo.x[1] - maxFaceInfo.x[0] + 1);
				int max_h = (int)(maxFaceInfo.y[1] - maxFaceInfo.y[0] + 1);
				int max_area = max_w * max_h;
				if (area > max_area)
				{
					maxFaceInfo = faceInfo;
				}
			}
		}
	}

	return maxFaceInfo;
}

void FaceInfoNcnn2FaceInfo(FaceInfo_ncnn& fi_ncnn, FaceInfo& fi) {
	fi.bbox.score = fi_ncnn.score;
	fi.bbox.xmin = fi_ncnn.x[0];
	fi.bbox.xmax = fi_ncnn.x[1];
	fi.bbox.ymin = fi_ncnn.y[0];
	fi.bbox.ymax = fi_ncnn.y[1];
	std::copy(std::begin(fi.bbox_reg), std::end(fi.bbox_reg), std::begin(fi_ncnn.regreCoord));
}

void FaceInfoNcnn2FaceInfo(vector<FaceInfo_ncnn>& fis_ncnn, vector<FaceInfo>& fis) {
	for (int i = 0; i < fis_ncnn.size(); i++) {
		FaceInfo fi;
		FaceInfoNcnn2FaceInfo(fis_ncnn[i], fi);
		fis.push_back(fi);
	}
}

void FaceInfo2FaceInfoNcnn(FaceInfo& fi, FaceInfo_ncnn& fi_ncnn) {
	fi_ncnn.score = fi.bbox.score;
	fi_ncnn.x[0] = fi.bbox.xmin;
	fi_ncnn.x[1] = fi.bbox.xmax;
	fi_ncnn.y[0] = fi.bbox.ymin;
	fi_ncnn.y[1] = fi.bbox.ymax;
	for (int i = 0; i < 10; i++) {
		fi_ncnn.landmark[i] = fi.landmark[i];
	}
}

void FaceInfo2FaceInfoNcnn(vector<FaceInfo>& fis, vector<FaceInfo_ncnn>& fis_ncnn) {
	for (int i = 0; i < fis.size(); i++) {
		FaceInfo_ncnn fi_ncnn;
		FaceInfo2FaceInfoNcnn(fis[i], fi_ncnn);
		fis_ncnn.push_back(fi_ncnn);
	}
}

bool isFacingCamera(FaceInfo& faceInfo) {
	int xmin = faceInfo.bbox.xmin;
	int ymin = faceInfo.bbox.ymin;
	int xmax = faceInfo.bbox.xmax;
	int ymax = faceInfo.bbox.ymax;
	int width = xmax - xmin;
	int height = ymax - ymin;
	Point2f center = Point2f(xmin + width / 2, ymin + height / 2);
	Point2f leftEye = Point2f(faceInfo.landmark[0], faceInfo.landmark[1]);
	Point2f rightEye = Point2f(faceInfo.landmark[2], faceInfo.landmark[3]);
	Point2f nose = Point2f(faceInfo.landmark[4], faceInfo.landmark[5]);
	Point2f leftMouth = Point2f(faceInfo.landmark[6], faceInfo.landmark[7]);
	Point2f rightMouth = Point2f(faceInfo.landmark[8], faceInfo.landmark[9]);

	int eyeDistance = abs(leftEye.x - rightEye.x);
	float ratio_x = eyeDistance * 1.0f / width;
	int mouth2EyeDistance = abs(leftMouth.y - leftEye.y);
	float ratio_y = mouth2EyeDistance * 1.0f / height;

	if (abs(nose.x - center.x) < 10) {
		return true;
	}

	/*if (abs(leftEye.y - rightEye.y) < 5 && ratio_x > 0.4 && ratio_y > 0.4) {
		return true;
	}*/
	return false;
}

MTCNN::MTCNN(const string& proto_model_dir) {
	//    PNet_ = cv::dnn::readNetFromCaffe(proto_model_dir + "/det1.prototxt", proto_model_dir + "/det1_half.caffemodel");
	PNet_ = cv::dnn::readNetFromCaffe(proto_model_dir + "/det1_.prototxt", proto_model_dir + "/det1_.caffemodel");

	RNet_ = cv::dnn::readNetFromCaffe(proto_model_dir + "/det2.prototxt", proto_model_dir + "/det2_half.caffemodel");
	ONet_ = cv::dnn::readNetFromCaffe(proto_model_dir + "/det3-half.prototxt", proto_model_dir + "/det3-half.caffemodel");
}


float MTCNN::IoU(float xmin, float ymin, float xmax, float ymax,
	float xmin_, float ymin_, float xmax_, float ymax_, bool is_iom) {
	float iw = std::min(xmax, xmax_) - std::max(xmin, xmin_) + 1;
	float ih = std::min(ymax, ymax_) - std::max(ymin, ymin_) + 1;
	if (iw <= 0 || ih <= 0)
		return 0;
	float s = iw * ih;
	if (is_iom) {
		float ov = s / min((xmax - xmin + 1)*(ymax - ymin + 1), (xmax_ - xmin_ + 1)*(ymax_ - ymin_ + 1));
		return ov;
	}
	else {
		float ov = s / ((xmax - xmin + 1)*(ymax - ymin + 1) + (xmax_ - xmin_ + 1)*(ymax_ - ymin_ + 1) - s);
		return ov;
	}
}
void MTCNN::BBoxRegression(vector<FaceInfo>& bboxes) {
	//#pragma omp parallel for num_threads(threads_num)
	for (int i = 0; i < bboxes.size(); ++i) {
		FaceBox &bbox = bboxes[i].bbox;
		float *bbox_reg = bboxes[i].bbox_reg;
		float w = bbox.xmax - bbox.xmin + 1;
		float h = bbox.ymax - bbox.ymin + 1;
		bbox.xmin += bbox_reg[0] * w;
		bbox.ymin += bbox_reg[1] * h;
		bbox.xmax += bbox_reg[2] * w;
		bbox.ymax += bbox_reg[3] * h;
	}
}
void MTCNN::BBoxPad(vector<FaceInfo>& bboxes, int width, int height) {
	//#pragma omp parallel for num_threads(threads_num)
	for (int i = 0; i < bboxes.size(); ++i) {
		FaceBox &bbox = bboxes[i].bbox;
		bbox.xmin = round(max(bbox.xmin, 0.f));
		bbox.ymin = round(max(bbox.ymin, 0.f));
		bbox.xmax = round(min(bbox.xmax, width - 1.f));
		bbox.ymax = round(min(bbox.ymax, height - 1.f));
	}
}
void MTCNN::BBoxPadSquare(vector<FaceInfo>& bboxes, int width, int height) {
	//#pragma omp parallel for num_threads(threads_num)
	for (int i = 0; i < bboxes.size(); ++i) {
		FaceBox &bbox = bboxes[i].bbox;
		float w = bbox.xmax - bbox.xmin + 1;
		float h = bbox.ymax - bbox.ymin + 1;
		float side = h > w ? h : w;
		bbox.xmin = round(max(bbox.xmin + (w - side)*0.5f, 0.f));

		bbox.ymin = round(max(bbox.ymin + (h - side)*0.5f, 0.f));
		bbox.xmax = round(min(bbox.xmin + side - 1, width - 1.f));
		bbox.ymax = round(min(bbox.ymin + side - 1, height - 1.f));
	}
}
void MTCNN::GenerateBBox(Mat* confidence, Mat* reg_box,
	float scale, float thresh) {
	int feature_map_w_ = confidence->size[3];
	int feature_map_h_ = confidence->size[2];
	int spatical_size = feature_map_w_ * feature_map_h_;
	//    const float* confidence_data = (float*)(confidence->data + spatical_size);

	std::cout << confidence->size;
	std::cout << " " << scale << std::endl;


	const float* confidence_data = (float*)(confidence->data);
	confidence_data += spatical_size;

	cv::Mat image(feature_map_h_, feature_map_w_, confidence->type());

	image.data = (unsigned  char*)(confidence_data);
	//    cv::imshow("image",image);
	//    cv::waitKey(0);




	//    std::cout<<confidence_data[0]<<std::endl;

	const float* reg_data = (float*)(reg_box->data);
	candidate_boxes_.clear();
	for (int i = 0; i < spatical_size; i++) {
		//        if (confidence_data[i] >= thresh) {
		if (confidence_data[i] <= 1 - thresh) {

			int y = i / feature_map_w_;
			int x = i - feature_map_w_ * y;
			FaceInfo faceInfo;
			FaceBox &faceBox = faceInfo.bbox;

			faceBox.xmin = (float)(x * pnet_stride) / scale;
			faceBox.ymin = (float)(y * pnet_stride) / scale;
			faceBox.xmax = (float)(x * pnet_stride + pnet_cell_size - 1.f) / scale;
			faceBox.ymax = (float)(y * pnet_stride + pnet_cell_size - 1.f) / scale;
			faceInfo.bbox_reg[0] = reg_data[i];
			faceInfo.bbox_reg[1] = reg_data[i + spatical_size];
			faceInfo.bbox_reg[2] = reg_data[i + 2 * spatical_size];
			faceInfo.bbox_reg[3] = reg_data[i + 3 * spatical_size];
			faceBox.score = confidence_data[i];
			candidate_boxes_.push_back(faceInfo);
		}
	}
}
std::vector<FaceInfo> MTCNN::NMS(std::vector<FaceInfo>& bboxes,
	float thresh, char methodType) {
	std::vector<FaceInfo> bboxes_nms;
	if (bboxes.size() == 0) {
		return bboxes_nms;
	}
	std::sort(bboxes.begin(), bboxes.end(), CompareBBox);

	int32_t select_idx = 0;
	int32_t num_bbox = static_cast<int32_t>(bboxes.size());
	std::vector<int32_t> mask_merged(num_bbox, 0);
	bool all_merged = false;

	while (!all_merged) {
		while (select_idx < num_bbox && mask_merged[select_idx] == 1)
			select_idx++;
		if (select_idx == num_bbox) {
			all_merged = true;
			continue;
		}

		bboxes_nms.push_back(bboxes[select_idx]);
		mask_merged[select_idx] = 1;

		FaceBox select_bbox = bboxes[select_idx].bbox;
		float area1 = static_cast<float>((select_bbox.xmax - select_bbox.xmin + 1) * (select_bbox.ymax - select_bbox.ymin + 1));
		float x1 = static_cast<float>(select_bbox.xmin);
		float y1 = static_cast<float>(select_bbox.ymin);
		float x2 = static_cast<float>(select_bbox.xmax);
		float y2 = static_cast<float>(select_bbox.ymax);

		select_idx++;
		//#pragma omp parallel for num_threads(threads_num)
		for (int32_t i = select_idx; i < num_bbox; i++) {
			if (mask_merged[i] == 1)
				continue;

			FaceBox & bbox_i = bboxes[i].bbox;
			float x = std::max<float>(x1, static_cast<float>(bbox_i.xmin));
			float y = std::max<float>(y1, static_cast<float>(bbox_i.ymin));
			float w = std::min<float>(x2, static_cast<float>(bbox_i.xmax)) - x + 1;
			float h = std::min<float>(y2, static_cast<float>(bbox_i.ymax)) - y + 1;
			if (w <= 0 || h <= 0)
				continue;

			float area2 = static_cast<float>((bbox_i.xmax - bbox_i.xmin + 1) * (bbox_i.ymax - bbox_i.ymin + 1));
			float area_intersect = w * h;

			switch (methodType) {
			case 'u':
				if (static_cast<float>(area_intersect) / (area1 + area2 - area_intersect) > thresh)
					mask_merged[i] = 1;
				break;
			case 'm':
				if (static_cast<float>(area_intersect) / std::min(area1, area2) > thresh)
					mask_merged[i] = 1;
				break;
			default:
				break;
			}
		}
	}
	return bboxes_nms;
}

vector<FaceInfo> MTCNN::NextStage(const cv::Mat& image, vector<FaceInfo> &pre_stage_res, int input_w, int input_h, int stage_num, const float threshold) {
	vector<FaceInfo> res;
	int batch_size = (int)pre_stage_res.size();
	if (batch_size == 0)
		return res;
	Mat* input_layer = nullptr;
	Mat* confidence = nullptr;
	Mat* reg_box = nullptr;
	Mat* reg_landmark = nullptr;

	std::vector< Mat > targets_blobs;



	switch (stage_num) {
	case 2: {
		//            input_layer = RNet_->input_blobs()[0];
		//            input_layer->Reshape(batch_size, 3, input_h, input_w);
		//            RNet_->Reshape();
	}break;
	case 3: {
		//            input_layer = ONet_->input_blobs()[0];
		//            input_layer->Reshape(batch_size, 3, input_h, input_w);
		//            ONet_->Reshape();
	}break;
	default:
		return res;
		break;
	}
	//    float * input_data = input_layer->mutable_cpu_data();
	int spatial_size = input_h * input_w;

	//#pragma omp parallel for num_threads(threads_num)

	std::vector<cv::Mat> inputs;

	for (int n = 0; n < batch_size; ++n) {
		FaceBox &box = pre_stage_res[n].bbox;
		Mat roi = image(Rect(Point((int)box.xmin, (int)box.ymin), Point((int)box.xmax, (int)box.ymax))).clone();
		resize(roi, roi, Size(input_w, input_h));
		inputs.push_back(roi);
		//resize好的face roi 里面
	}

	//
//    cv::Mat inputBlob = cv::dnn::blobFromImage(resized, std_val,cv::Size(),mean_val);

//    cv::imshow("image",inputs[0]);
//    cv::waitKey(0);


	Mat blob_input = dnn::blobFromImages(inputs, std_val, cv::Size(), cv::Scalar(mean_val, mean_val, mean_val), false);

	//    PNet_.setInput(inputBlob, "data");
	//    const std::vector< String >  targets_node{"conv4-2","prob1"};
	//    std::vector< Mat > targets_blobs;
	//    PNet_.forward(targets_blobs,targets_node);

	switch (stage_num) {
	case 2: {
		RNet_.setInput(blob_input, "data");
		const std::vector< String >  targets_node{ "conv5-2","prob1" };
		RNet_.forward(targets_blobs, targets_node);
		confidence = &targets_blobs[1];
		reg_box = &targets_blobs[0];

		float* confidence_data = (float*)confidence->data;
	}break;
	case 3: {

		ONet_.setInput(blob_input, "data");
		const std::vector< String >  targets_node{ "conv6-2","conv6-3","prob1" };
		ONet_.forward(targets_blobs, targets_node);
		reg_box = &targets_blobs[0];
		reg_landmark = &targets_blobs[1];
		confidence = &targets_blobs[2];

	}break;
	}


	const float* confidence_data = (float*)confidence->data;
	//    std::cout<<"confidence_data[0] "<<confidence_data[0]<<std::endl;

	const float* reg_data = (float*)reg_box->data;
	const float* landmark_data = nullptr;
	if (reg_landmark) {
		landmark_data = (float*)reg_landmark->data;
	}
	for (int k = 0; k < batch_size; ++k) {
		if (confidence_data[2 * k + 1] >= threshold) {
			FaceInfo info;
			info.bbox.score = confidence_data[2 * k + 1];
			info.bbox.xmin = pre_stage_res[k].bbox.xmin;
			info.bbox.ymin = pre_stage_res[k].bbox.ymin;
			info.bbox.xmax = pre_stage_res[k].bbox.xmax;
			info.bbox.ymax = pre_stage_res[k].bbox.ymax;
			for (int i = 0; i < 4; ++i) {
				info.bbox_reg[i] = reg_data[4 * k + i];
			}
			if (reg_landmark) {
				float w = info.bbox.xmax - info.bbox.xmin + 1.f;
				float h = info.bbox.ymax - info.bbox.ymin + 1.f;
				for (int i = 0; i < 5; ++i) {
					info.landmark[2 * i] = landmark_data[10 * k + 2 * i] * w + info.bbox.xmin;
					info.landmark[2 * i + 1] = landmark_data[10 * k + 2 * i + 1] * h + info.bbox.ymin;
				}
			}
			res.push_back(info);
		}
	}
	return res;
}

vector<FaceInfo> MTCNN::ProposalNet(const cv::Mat& img, int minSize, float threshold, float factor) {
	cv::Mat  resized;
	int width = img.cols;
	int height = img.rows;
	float scale = 12.f / minSize;
	float minWH = std::min(height, width) *scale;
	std::vector<float> scales;
	while (minWH >= 12) {
		scales.push_back(scale);
		minWH *= factor;
		scale *= factor;
	}

	//    Mat* input_layer = PNet_->input_blobs()[0];
	total_boxes_.clear();
	for (int i = 0; i < scales.size(); i++) {
		int ws = (int)std::ceil(width*scales[i]);
		int hs = (int)std::ceil(height*scales[i]);
		cv::resize(img, resized, cv::Size(ws, hs), 0, 0, cv::INTER_LINEAR);
		//
		//        input_layer->Reshape(1, 3, hs, ws);
		//        PNet_->Reshape();
		//
		//        float * input_data = input_layer->mutable_cpu_data();
		//        cv::Vec3b * img_data = (cv::Vec3b *)resized.data;
		//        int spatial_size = ws* hs;
		//        for (int k = 0; k < spatial_size; ++k) {
		//            input_data[k] = float((img_data[k][0] - mean_val)* std_val);
		//            input_data[k + spatial_size] = float((img_data[k][1] - mean_val) * std_val);
		//            input_data[k + 2 * spatial_size] = float((img_data[k][2] - mean_val) * std_val);
		//        }


		cv::Mat inputBlob = cv::dnn::blobFromImage(resized, 1 / 255.0, cv::Size(), cv::Scalar(0, 0, 0), false);

		float* c = (float*)inputBlob.data;
		PNet_.setInput(inputBlob, "data");
		const std::vector< cv::String >  targets_node{ "conv4-2","prob1" };
		std::vector< cv::Mat > targets_blobs;
		PNet_.forward(targets_blobs, targets_node);

		cv::Mat prob = targets_blobs[1]
			;
		cv::Mat reg = targets_blobs[0];
		GenerateBBox(&prob, &reg, scales[i], threshold);
		//
		std::vector<FaceInfo> bboxes_nms = NMS(candidate_boxes_, 0.5, 'u');
		if (bboxes_nms.size() > 0) {
			total_boxes_.insert(total_boxes_.end(), bboxes_nms.begin(), bboxes_nms.end());
		}
	}
	int num_box = (int)total_boxes_.size();
	//    std::cout<<num_box<<std::endl;

	vector<FaceInfo> res_boxes;
	if (num_box != 0) {
		res_boxes = NMS(total_boxes_, 0.7f, 'u');
		BBoxRegression(res_boxes);
		BBoxPadSquare(res_boxes, width, height);
	}
	return res_boxes;
}

vector<FaceInfo> MTCNN::Detect_mtcnn(const cv::Mat& image, const int minSize, const float* threshold, const float factor, const int stage) {
	vector<FaceInfo> pnet_res;
	vector<FaceInfo> rnet_res;
	vector<FaceInfo> onet_res;
	if (stage >= 1) {
		pnet_res = ProposalNet(image, minSize, threshold[0], factor);
	}
	if (stage >= 2 && pnet_res.size() > 0) {
		if (pnet_max_detect_num < (int)pnet_res.size()) {
			pnet_res.resize(pnet_max_detect_num);
		}
		int num = (int)pnet_res.size();
		int size = (int)ceil(1.f*num / step_size);
		for (int iter = 0; iter < size; ++iter) {
			int start = iter * step_size;
			int end = min(start + step_size, num);
			vector<FaceInfo> input(pnet_res.begin() + start, pnet_res.begin() + end);
			vector<FaceInfo> res = NextStage(image, input, 24, 24, 2, threshold[1]);
			rnet_res.insert(rnet_res.end(), res.begin(), res.end());
		}
		rnet_res = NMS(rnet_res, 0.4f, 'm');
		BBoxRegression(rnet_res);
		BBoxPadSquare(rnet_res, image.cols, image.rows);

	}
	if (stage >= 3 && rnet_res.size() > 0) {
		int num = (int)rnet_res.size();
		int size = (int)ceil(1.f*num / step_size);
		for (int iter = 0; iter < size; ++iter) {
			int start = iter * step_size;
			int end = min(start + step_size, num);
			vector<FaceInfo> input(rnet_res.begin() + start, rnet_res.begin() + end);
			vector<FaceInfo> res = NextStage(image, input, 48, 48, 3, threshold[2]);
			onet_res.insert(onet_res.end(), res.begin(), res.end());
		}
		BBoxRegression(onet_res);
		onet_res = NMS(onet_res, 0.4f, 'm');
		BBoxPad(onet_res, image.cols, image.rows);

	}
	if (stage == 1) {
		return pnet_res;
	}
	else if (stage == 2) {
		return rnet_res;
	}
	else if (stage == 3) {
		return onet_res;
	}
	else {
		return onet_res;
	}
}