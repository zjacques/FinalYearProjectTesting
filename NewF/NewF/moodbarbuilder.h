#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

class MoodbarBuilder {
public:
	MoodbarBuilder();

	void Init(int bands, int rate_hz);
	void AddFrame(const double* magnitudes, std::size_t size);
	std::vector<std::uint8_t> Finish(int width);

private:
	struct Rgb {
		Rgb() : r(0), g(0), b(0) {}
		Rgb(double r_, double g_, double b_) : r(r_), g(g_), b(b_) {}

		double r, g, b;
	};

	int BandFrequency(int band) const;
	static void Normalize(std::vector<Rgb>* vals, double Rgb::*member);

	std::vector<unsigned int> barkband_table_;
	int bands_;
	int rate_hz_;

	std::vector<Rgb> frames_;
};