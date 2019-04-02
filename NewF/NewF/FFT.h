#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <fstream>
#include <complex>
#include <valarray>
#include <math.h>
#include <algorithm>
#include <numeric>
#include <queue>
#include "dr_flac.h"
#include "moodbarbuilder.h"

const double PI = 3.141592653589793238460 ;

using namespace std ;
using namespace sf ;

typedef complex<double> Complex;
typedef valarray<Complex> CArray;

class FFT
{
public:
	FFT(string const& _path,int const& _bufferSize);

	void hammingWindow() ;
	void waveForm();
	void fft(CArray &x) ;

	void moodBar();

	void beatDetect();
	void update() ;

	void bars(float const& max) ;
	void lines(float const& max) ;

	void draw(RenderWindow &window) ;

private:
	ostringstream outputString;

	MoodbarBuilder moodbar;
	string path ;
	SoundBuffer buffer ;
	Sound sound ;

	deque<double> livehistory;
	int livebeats = 0;
	float liveC = 1.3;
	int lastBeats = 0;
	int lastSecond = 0;
	deque<double> bpshistory;

	queue<float> beatTimes;
	map<float, unsigned int> m_beatHistory;

	float getMostCommonBPM();

	vector<Complex> sample ;
	vector<float> window ;
	vector<float> shortWindow;
	CArray bin ;

	VertexArray VA1 ;
	VertexArray VA2 ;
	VertexArray VA3 ;
	VertexArray VA4;
	vector<Vertex> cascade ;
	
	int sampleRate ;
	int sampleCount ;
	int bufferSize ;
	int mark ;

	int max;

	struct colourTriplet {
		double red;
		double green;
		double blue;
	};

};

