#include "tfmodel.h"

#ifndef EXCLUDE_TFLITE


void loadObjectLabels(const std::string& f, std::vector<std::string>& objs) {
	objs.clear();
	std::ifstream file(f);
	std::string line;
	int32_t start, end;
	std::getline(file, line, '\n');
	if(line == "item {") {
		while(std::getline(file, line, '\n')) {
			start = end = -1;
			for (size_t i = 0; i < line.size(); i++) {
				if (line.at(i) == '"') {
					if (start < 0) {
						start = i + 1;
					}
					else {
						end = i;
						i = line.size();	// break
					}
				}
			}
			if (start >= 0 && end >= 0) {
				objs.emplace_back(std::move(line.substr(start, end - start)));
			}
		}
	} else {
		objs.push_back(line);
		while(std::getline(file, line, '\n')) {
			objs.push_back(line);
		}
	}
	file.close();
}

TfModel::TfModel(std::initializer_list<std::pair<const char*, Optimization> > models, size_t th) {

	for(auto item = models.begin(); item != models.end(); item++) {
		this->map = tflite::FlatBufferModel::BuildFromFile(item->first);
		if(this->map) {
			if(item->second == Optimization::EDGETPU && tpusAvailable()) {	// switch-case if more optimization options
				this->resolver.AddCustom(edgetpu::kCustomOp, edgetpu::RegisterCustomOp());
				tflite::InterpreterBuilder builder(*this->map, this->resolver);
				builder.SetNumThreads(th);
				builder(&this->model);
				this->edgetpu_context = edgetpu::EdgeTpuManager::GetSingleton()->OpenDevice();
				this->model->SetExternalContext(kTfLiteEdgeTpuContext, this->edgetpu_context.get());
			} else {
				tflite::InterpreterBuilder builder(*this->map, this->resolver);
				builder.SetNumThreads(th);
				builder(&this->model);
			}
			if(this->isValid()) {
				break;
			}
		}
	}

	if(this->isValid()) {
		this->model->AllocateTensors();
		if(this->model->inputs().size() == 1) {
			TfLiteTensor* input = this->model->input_tensor(0);
			TfLiteIntArray* dims = input->dims;
			this->input_size = cv::Size(dims->data[1], dims->data[2]);
			if(dims->data[3] == 3 && input->type == kTfLiteUInt8) {
				this->input_tensor = cv::Mat(this->input_size, CV_8UC3, input->data.data);
			}
		} else {
			this->input_size = cv::Size(-1, -1);
		}
	}

}

#endif