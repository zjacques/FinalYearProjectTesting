#include "moodbarbuilder.h"

#include <algorithm>
#include <cmath>

namespace {

	static const int sBarkBands[] = {
		100,  200,  300,  400,  510,  630,  770,  920,  1080, 1270, 1480,  1720,
		2000, 2320, 2700, 3150, 3700, 4400, 5300, 6400, 7700, 9500, 12000, 15500 };

	static const int sBarkBandCount = std::extent<decltype(sBarkBands)>::value;

}  // namespace

MoodbarBuilder::MoodbarBuilder() : bands_(0), rate_hz_(0) {}

int MoodbarBuilder::BandFrequency(int band) const {
	return ((rate_hz_ / 2) * band + rate_hz_ / 4) / bands_;
}

void MoodbarBuilder::Init(int bands, int rate_hz) {
	bands_ = bands;
	rate_hz_ = rate_hz;

	barkband_table_.clear();
	barkband_table_.reserve(bands + 1);

	int barkband = 0;
	for (int i = 0; i < bands + 1; ++i) {
		if (barkband < sBarkBandCount - 1 &&
			BandFrequency(i) >= sBarkBands[barkband]) {
			barkband++;
		}

		barkband_table_.push_back(barkband);
	}
}

void MoodbarBuilder::AddFrame(const double* magnitudes, std::size_t size) {
	if (size > barkband_table_.size()) {
		return;
	}

	// Calculate total magnitudes for different bark bands.
	double bands[sBarkBandCount];
	for (std::size_t i = 0; i < sBarkBandCount; ++i) {
		bands[i] = 0.0;
	}

	for (std::size_t i = 0; i < size; ++i) {
		bands[barkband_table_[i]] += magnitudes[i];
	}

	// Now divide the bark bands into thirds and compute their total amplitudes.
	double rgb[] = { 0, 0, 0 };
	for (std::size_t i = 0; i < sBarkBandCount; ++i) {
		rgb[(i * 3) / sBarkBandCount] += bands[i] * bands[i];
	}

	frames_.push_back(Rgb(sqrt(rgb[0]), sqrt(rgb[1]), sqrt(rgb[2])));
}

void MoodbarBuilder::Normalize(std::vector<Rgb>* vals, double Rgb::*member) {
	double mini = vals->at(0).*member;
	double maxi = vals->at(0).*member;
	for (std::size_t i = 1; i < vals->size(); i++) {
		const double value = vals->at(i).*member;
		if (value > maxi) {
			maxi = value;
		}
		else if (value < mini) {
			mini = value;
		}
	}

	double avg = 0;
	int t = 0;
	for (const Rgb& rgb : *vals) {
		const double value = rgb.*member;
		if (value != mini && value != maxi) {
			avg += value / vals->size();
			t++;
		}
	}

	double tu = 0;
	double tb = 0;
	double avgu = 0;
	double avgb = 0;
	for (const Rgb& rgb : *vals) {
		const double value = rgb.*member;
		if (value != mini && value != maxi) {
			if (value > avg) {
				avgu += value;
				tu++;
			}
			else {
				avgb += value;
				tb++;
			}
		}
	}
	avgu /= tu;
	avgb /= tb;

	tu = 0;
	tb = 0;
	double avguu = 0;
	double avgbb = 0;
	for (const Rgb& rgb : *vals) {
		const double value = rgb.*member;
		if (value != mini && value != maxi) {
			if (value > avgu) {
				avguu += value;
				tu++;
			}
			else if (value < avgb) {
				avgbb += value;
				tb++;
			}
		}
	}
	avguu /= tu;
	avgbb /= tb;

	mini = std::max(avg + (avgb - avg) * 2, avgbb);
	maxi = std::min(avg + (avgu - avg) * 2, avguu);
	double delta = maxi - mini;
	if (delta == 0) {
		delta = 1;
	}

	for (auto it = vals->begin(); it != vals->end(); ++it) {
		double* value = &((*it).*member);
		*value =
			std::isfinite(*value) ? std::max(0.0, std::min((*value - mini) / delta, 1.0)) : 0;
	}
}

std::vector<std::uint8_t> MoodbarBuilder::Finish(int width) {
	std::vector<std::uint8_t> ret;
	ret.resize(width * 3);
	std::uint8_t* data = ret.data();
	if (frames_.size() == 0) return ret;

	Normalize(&frames_, &Rgb::r);
	Normalize(&frames_, &Rgb::g);
	Normalize(&frames_, &Rgb::b);

	for (int i = 0; i < width; ++i) {
		Rgb rgb;
		int start = i * frames_.size() / width;
		int end = (i + 1) * frames_.size() / width;
		if (start == end) {
			end = start + 1;
		}

		for (int j = start; j < end; j++) {
			const Rgb& frame = frames_[j];
			rgb.r += frame.r * 255;
			rgb.g += frame.g * 255;
			rgb.b += frame.b * 255;
		}

		const int n = end - start;

		*(data++) = rgb.r / n;
		*(data++) = rgb.g / n;
		*(data++) = rgb.b / n;
	}
	return ret;
}