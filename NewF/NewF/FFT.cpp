#include "FFT.h"

FFT::FFT(string const& _path,int const& _bufferSize)
{
	path = _path ;
	if(!buffer.loadFromFile(path)) std::cout<<"Unable to load buffer"<<endl ;
	songname = path.substr(0, path.find("."));
	songname = songname.substr(songname.find("/")+1);

	int channels = buffer.getChannelCount();
	sampleRate = buffer.getSampleRate()*buffer.getChannelCount() ;
	sampleCount = buffer.getSampleCount() ;

	//Buffersize is going to be the number of samples every loop runs through
	if(_bufferSize < sampleCount) bufferSize = _bufferSize ;
	else bufferSize = sampleCount ;
	mark = 0 ;//mark is the mark of the song in samples you are currently at, listening in realtime

	//windowing function: makes a window array to scale every run through of the buffer
	for (int i = 0; i < bufferSize; i++)
	{
		window.push_back(0.54 - 0.46*cos(2 * PI*i / (float)bufferSize));
	}

	//windowing function: makes a window array to scale every run through of the buffer
	for (int i = 0; i < bufferSize/2; i++)
	{
		shortWindow.push_back(0.54 - 0.46*cos(2 * PI*i / ((float)bufferSize/2)));
	}
		//window.push_back(1.0f);
		//window.push_back(0.54-0.46*cos(2*PI*i/(float)bufferSize)) ;

	sample.resize(bufferSize) ;
}

//applies the window function to the buffer of samples (sample[])
//sample seems like a terrible name
//also makes each "sample" into a complex number in prep for the actual fft.
void FFT::hammingWindow()
{
	mark = sound.getPlayingOffset().asSeconds()*sampleRate;
	//if the next loop isn't going to overflow the song's end
	if (mark + bufferSize < sampleCount)
	{
		//start at mark, run forward up to buffersize
		for (int i = mark; i < bufferSize + mark; i++)
		{
			sample[i - mark] = Complex(buffer.getSamples()[i] * window[i - mark], 0);
		}
	}
}

//runs through every sample in the buffer(the whole file) and plots points across it at intervals of the buffer size.
//No windows, just selected samples and their basic magnitudes
void FFT::waveForm()
{
	waveStream << ",\n\"m\":[";
	int lastmark = 1;

	int numpoints = sampleCount / bufferSize;
	for (int i = 0; i < numpoints; i++)
	{
		float x = (float)i / (float)numpoints;
		float n = x * (float)sampleCount;
		int max = 0;
		int min = 0;
		for (int j = lastmark; j < (int)n; j++)
		{
			if ((int)buffer.getSamples()[j] >= 0)
				max += (int)buffer.getSamples()[j];
			else
				min += (int)buffer.getSamples()[j];
		}
		Int16 sm = (int)buffer.getSamples()[i];
		if (((int)n - lastmark) / 2 != 0)
		{
			max /= (((int)n - lastmark) / 2);
			min /= ((((int)n - lastmark)) / 2);
		}
		else {
			max = min = 0;
		}
		waveStream << max << "," << min;
		lastmark = (int)n;
		if (i != (numpoints-1))
			waveStream << ",\n";
	}
	waveStream << "]\n";
	std::cout << "wave made" << std::endl;
}

//The fft function I got from this code sample.
void FFT::fft(CArray &x)
{
	const int N = x.size();
	if(N <= 1) return;

	CArray even = x[slice(0,N/2,2)];
	CArray  odd = x[slice(1,N/2,2)];

	fft(even);
	fft(odd);

	for(int k = 0 ; k < N/2 ; k++)
	{
		Complex t = polar(1.0,-2 * PI * k / N) * odd[k];
		x[k] = even[k] + t;
		x[k+N/2] = even[k] - t;
	}
}

//Resets the output string and opens it for JSON
void FFT::beginString()
{
	outputString.clear(); 
	outputString << "{";
}

//Adds all the output strings together into one and closes the JSON
void FFT::endString()
{
	outputString << bmpStream.str();
	outputString << waveStream.str();
	outputString << colStream.str();
	outputString << "}";
}

//Prints the output string to a file with the same name as the audio file
//Please use beginString and endString to insert your data to the string before printing
void FFT::printToFile()
{
	ofstream myfile;
	myfile.open(songname + ".txt");
	myfile << outputString.str();
	myfile.close();
}

//Does the whole window-ham-fft process over the whole song
//Then creates a coloured bar based on the spectrum mix of that loop's samples.
//Each coloured bar should represent one block of bufferSize length
void FFT::moodBar()
{
		//hamm

		//start at 0 sample
		//get and hamm samples 0-buffersize
		//fft those and store them
		//go to 0 sample + buffersize/2
		//repeat until sampleCount  

	//getting the red over time, comparing instant values with history, could work for getting beats?

	int startSample = 0;
	int currentSample = 0;
	CArray bin2;
	vector<int> barks;
	vector<colourTriplet> colors;
	while (currentSample < sampleCount-(bufferSize / 2))
	{
		//hamm
		for (currentSample = startSample; currentSample < startSample + bufferSize; currentSample++)
		{
			if (currentSample < sampleCount)
			{
				sample[currentSample-startSample] = Complex(buffer.getSamples()[currentSample] * window[currentSample-startSample], 0);
			}
		}
		//hammed?

		bin2 = CArray(sample.data(), bufferSize);

		//fft
		fft(bin2);
		//fft?

		float b = bin2.size();
		float barBand = sampleRate / bin2.size();

		//frequency?

		float bassMax = 200 / barBand;//the last line that should be in the bass band
		float midMax = 5000 / barBand;//the last mid line
		colourTriplet cols;
		int redCount = 0;
		int greenCount = 0;
		int blueCount = 0;
		cols.red = 0;
		cols.green = 0;
		cols.blue = 0 ;
		for (float i = 1; i < bin2.size(); i++)
		{
			float amplitude = abs(bin2[(int)i]);

			if (i < bassMax)
			{
				cols.red += amplitude;
				redCount++;

			}
			else if (i < midMax)
			{
				cols.green += amplitude;
				greenCount++;
			}
			else
			{
				cols.blue += amplitude;
				blueCount++;
			}
		}

		cols.red /= redCount;
		cols.green /= greenCount;
		cols.blue /= blueCount;

		colors.push_back(cols);
		//startSample += bufferSize/4;
		startSample += bufferSize;
	}

	//draw the actual moodbar
	waveStream << ",\"colours\":[\n";
	//issue is: there are far more bars in the higher bands than the bass, so it will always skew blue
	for (float i = 0; i < colors.size(); i++) {
		double maxVal = std::max(colors[i].red, colors[i].green);
		maxVal = std::max(maxVal, colors[i].blue);
		if (maxVal == 0)
		{
			colors[i].red = maxVal;
			colors[i].green = maxVal;
			colors[i].blue = maxVal;
		}
		else
		{
			colors[i].red /= maxVal;
			colors[i].green /= maxVal;
			colors[i].blue /= maxVal;
		}
		//color should be normalized now? Skews Red now?
		waveStream << "{\"R\":" << colors[i].red << ",\"G\":" << colors[i].green << ",\"B\":" << colors[i].blue << "}";
		if(i!=colors.size()-1)
			waveStream << ",\n";
	}
	waveStream << "]\n";
	std::cout << "color samples" << colors.size() << endl;
	std::cout << "moodbar made" << endl;

}

void FFT::beatDetect()
{
	//hamm

	//start at 0 sample
	//get and hamm samples 0-buffersize
	//fft those and store them
	//go to 0 sample + buffersize/2
	//repeat until sampleCount  

	int startSample = 0;
	int currentSample = 0;
	CArray bin2;
	deque<double> history(sampleRate / (bufferSize), 0.0);
	float C = 1.5;
	int beats = 0;
	//for each second in the song
	for (int sec = 0; sec < sampleCount; sec += sampleRate)
	{
		int second = sec / sampleRate;
		//for each window in each second
		while (currentSample < sec)
		{
			//hamm
			for (currentSample = startSample; currentSample < startSample + (bufferSize); currentSample++)
			{
				if (currentSample < sampleCount)
				{
					sample[currentSample - startSample] = Complex((buffer.getSamples()[currentSample]) * window[currentSample - startSample], 0);
				}
			}
			//hammed?
			bin2 = CArray(sample.data(), bufferSize);

			//fft
			fft(bin2);
			//fft?

			float b = bin2.size();
			//how many frequencies are in each bin
			float barBand = sampleRate / bin2.size();

			//grab bass

			float bassMax = 200 / barBand;//the last line that should be in the bass band
			double instantEnergy = 0.0f; //variable e in the equations

											//for each sample in window
			for (float i = 1; i < bassMax; i++)
			{
				float amplitude = abs(bin2[(int)i]);

				instantEnergy += amplitude * amplitude;
			}

			//average of history
			if (history.size() > 0)
			{
				double E = 0.0;
				for (auto x : history)  E += x;
				E /= history.size();

				double V = 0.0;
				for (auto x : history)  V += (x - E);

				C = (-0.0025714*V) + 1.5142857;

				if (instantEnergy > E*C && history.back() <= E * C)
				{
					beats++;
					double sthugg = ((double)currentSample / (double)sampleRate);
					beatTimes.push( sthugg);
					auto gguhts = ((double)currentSample / (double)sampleRate) - beatTimes.front();
					while (gguhts >= 1)
					{
						beatTimes.pop();
						if (beatTimes.empty()) break;
					}
				}
			}
			history.push_back(instantEnergy);
			
			if (history.size() > sampleRate/(bufferSize/4))
			{
				history.pop_front();
			}

			if (beatTimes.size() >= 2)
			{
				double ms = (double)(beatTimes.back() - beatTimes.front()) / ((double)beatTimes.size() - 1);
				double est = 60.0 / ms;
				float roundedEst = (float)(floor(est));
				if (roundedEst > 200)
					roundedEst /= 2;
				if (m_beatHistory.find(roundedEst) != m_beatHistory.end())
				{
					++m_beatHistory[roundedEst];
				}
				else {
					m_beatHistory.insert(make_pair(roundedEst, 1));
				}
			}
			
			startSample += (bufferSize/2);
			
		}
	}
	std::cout << "BPM is about : " << getMostCommonBPM() << std::endl;
	bmpStream << "\"BPM\": " << getMostCommonBPM() << "\n";
}

float FFT::getMostCommonBPM()
{
	float bestGuessBpm = 0.f;
	unsigned int highestCount = 0;
	for (std::map< float, unsigned int >::const_iterator iter = m_beatHistory.begin(); iter != m_beatHistory.end(); ++iter)
	{
		if (iter->second > highestCount)
		{
			bestGuessBpm = iter->first;
			highestCount = iter->second;
		}
	}

	return bestGuessBpm;
}
