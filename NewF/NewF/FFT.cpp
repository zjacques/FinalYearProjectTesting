#include "FFT.h"

FFT::FFT(string const& _path,int const& _bufferSize)
{
	path = _path ;
	if(!buffer.loadFromFile(path)) std::cout<<"Unable to load buffer"<<endl ;
	songname = path.substr(0, path.find("."));
	songname = songname.substr(songname.find("/")+1);
	drflac file;

	//sound.setBuffer(buffer) ;
	//sound.setLoop(true);
	//sound.play();

	VA1.setPrimitiveType(LineStrip) ;
	VA2.setPrimitiveType(Lines) ;
	VA3.setPrimitiveType(LineStrip) ;
	VA4.setPrimitiveType(Lines);
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
	VA1.resize(2000);
	//outputString << "{";

	/*beatDetect();
	waveForm();
	moodBar();*/


	//outputString << "}";
	/*ofstream myfile;
	myfile.open(songname+".txt");
	myfile << outputString.str();
	myfile.close();*/
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
			//VA1[i - mark] = Vertex(Vector2f(20, 250) + Vector2f((i - mark) / (float)bufferSize * 700, sample[i - mark].real()*0.005), Color::Color(255, 0, 0, 50));
		}
	}
}

//runs through every sample in the buffer(the whole file) and plots 1000 points across it. no windows, just selected samples and their
// basic magnitudes
//It's not pretty.
void FFT::waveForm()
{
	waveStream << ",\n\"m\":[";
	int lastmark = 1;
	//while (currentSample < sampleCount-(bufferSize / 2))
	//{
	//for (int i = 0; i < 1000; i++)
	int numpoints = sampleCount / bufferSize;
	for (int i = 0; i < numpoints; i++)
	{
		//sample[i - mark] = Complex(buffer.getSamples()[i], 0);
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
			//max = std::max(max, (int)buffer.getSamples()[j]);
			//min = std::min(min, (int)buffer.getSamples()[j]);
			//avg += buffer.getSamples()[j];
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
		//VA1[i] = Vertex(Vector2f(20, 250) + Vector2f(i / (float)numpoints * 700, sm*0.004), Color::Color(255, 0, 0, 255));
		//VA1[mark] = Vertex(Vector2f(20, 250) + Vector2f((mark) / (float)bufferSize * 700, sample[mark].real()*0.005), Color::Color(255, 0, 0, 50));
	}
	waveStream << "]\n";
	std::cout << "wave made" << std::endl;
}

//The fft function I got from this code sample.
//Haven't touched it
//Here there be dragons
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

void FFT::beginString()
{
	outputString.clear(); 
	outputString << "{";
}

void FFT::endString()
{
	outputString << bmpStream.str();
	outputString << waveStream.str();
	outputString << colStream.str();
	outputString << "}";
}

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

		VA4.append(Vertex(Vector2f((i/colors.size())*800 + 50, 400), Color(255/colors[(int)i].red, 255 / colors[(int)i].green, 255 / colors[(int)i].blue)));
		VA4.append(Vertex(Vector2f((i/colors.size()) * 800 + 50, 500), Color(255 / colors[(int)i].red, 255 / colors[(int)i].green, 255 / colors[(int)i].blue)));
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
					//std::cout << getMostCommonBPM() << std::endl;
					auto gguhts = ((double)currentSample / (double)sampleRate) - beatTimes.front();
					while (gguhts >= 1)
					{
						beatTimes.pop();
						if (beatTimes.empty()) break;
					}
					//std::cout << "beat " << beats << "\n";
					//std::cout << "threshold " << C << "\n";
					//double bpm = livebeats / (sound.getPlayingOffset().asSeconds() / 60);
					//std::cout << "bpm " << floor(bpm) << "\n";

				}
			}
			history.push_back(instantEnergy);
			//if(history is too long(greater than 1 second should be)) then remove one from it
			if (history.size() > sampleRate/(bufferSize/4))
			{
				history.pop_front();
			}

			if (beatTimes.size() >= 2)
			{
				double ms = (double)(beatTimes.back() - beatTimes.front()) / ((double)beatTimes.size() - 1);
				double est = 60.0 / ms;
				//float roundedEst = (float)(floor(est * 100.f) / 100.f);
				float roundedEst = (float)(floor(est));
				if (roundedEst > 200)
					roundedEst /= 2;
				//cout << roundedEst << endl;
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
		//cout << second << endl;
	}
	std::cout << "BPM is about : " << getMostCommonBPM() << std::endl;
	bmpStream << "\"BPM\": " << getMostCommonBPM() << "\n";
	/*float duration = buffer.getDuration().asSeconds();
	duration /= 60;
	double bpm = (double)beats / (duration);
	std::cout << "BPM is about : " << bpm << endl;*/
}

void FFT::update()
{
	hammingWindow();

	VA2.clear() ;
	//VA3.clear() ;

	bin = CArray(sample.data(),bufferSize) ;
	fft(bin) ;
	float max = 100000000;
	
	//lines(max) ;
	bars(max);
}

void FFT::bars(float const& max)
{
	float b = bin.size();
	float barBand = sampleRate / bin.size();
	float bassMax = 200 / barBand;//the last line that should be in the bass band
	float midMax = 5000 / barBand;//the last mid line

	double instantEnergy = 0.0f; //variable e in the equations
								 //for each sample in window
	for (float i = 1; i < bassMax; i++)
	{
		float amplitude = abs(bin[(int)i]);

		instantEnergy += amplitude * amplitude;
	}
	//average of history
	if (livehistory.size() > 0)
	{
		double E = 0.0;
		for (auto x : livehistory)  E += x;
		E /= livehistory.size();

		double V = 0.0;
		for (auto x : livehistory)  V += (x - E);

		liveC = (-0.0025714*V) + 1.5142857;
		//std::cout << "C" << C << "\n";
		//double E = accumulate(history.begin(), history.end(), 0.0) / history.size();
		//cout << "average " << E*C << endl;
		//cout << "instant " << instantEnergy << endl;
		if (instantEnergy > E*liveC && livehistory.back() <= E * liveC)
		{
			livebeats++;

			beatTimes.push(sound.getPlayingOffset().asMicroseconds());
			std::cout << getMostCommonBPM() << std::endl;

			while (sound.getPlayingOffset().asMicroseconds() - beatTimes.front() >= 1000000)
			{
				beatTimes.pop();
				if (beatTimes.empty()) break;
			}
			std::cout << "beat " << livebeats << "\n";
			//std::cout << "threshold " << liveC << "\n";
			//cout << sound.getPlayingOffset().asSeconds() << endl;
			//double bpm = livebeats / (sound.getPlayingOffset().asSeconds() / 60);
			//std::cout << "bpm " << floor(bpm) << "\n";
			/*bpshistory.push_back(bpm);
			for (auto x : bpshistory)  bpm += x;
			bpm /= bpshistory.size();
			std::cout << "bpm avg " << bpm << "\n";*/
			//beat!
		}
	}
	if (beatTimes.size() >= 2)
	{
		double ms = (double)(beatTimes.back() - beatTimes.front()) / ((double)beatTimes.size() - 1);
		double est = 60000000.0 / ms;
		float roundedEst = (float)(floor(est * 100.f) / 100.f);
		//cout << roundedEst << endl;
		if (m_beatHistory.find(roundedEst) != m_beatHistory.end())
		{
			++m_beatHistory[roundedEst];
		}
		else {
			m_beatHistory.insert(make_pair(roundedEst, 1));
		}
		/*if (m_beatHistory.find(roundedEst/2) != m_beatHistory.end())
		{
			//++m_beatHistory[roundedEst];
			++m_beatHistory[roundedEst/2];
		}
		if (m_beatHistory.find(roundedEst*2) != m_beatHistory.end())
		{
			++m_beatHistory[roundedEst * 2];
		}*/
	}
	livehistory.push_back(instantEnergy);

	//if(history is too long(greater than 1 second should be)) then remove one from it
	if (livehistory.size() > sampleRate / (bufferSize/4))
	{
		livehistory.pop_front();
	}

	/*if (bpshistory.size() > 100)
	{
		bpshistory.pop_front();
	}
	*/
	/*if ((int)sound.getPlayingOffset().asSeconds() > lastSecond)
	{
		double bps = (livebeats - lastBeats);
		bpshistory.push_back(bps);
		double bpm = 0;
		bpm /= bpshistory.size();
		std::cout << "bpm " << bpm*60 << "\n";
		lastBeats = livebeats;
		lastSecond = sound.getPlayingOffset().asSeconds();
	}*/

	VA2.setPrimitiveType(Lines) ;
	Vector2f position(0,800) ;
	//sampleRate
	//Bass = 0-250hz, mid = 251-5khz, treble = 5k-20k
	//width of bin bar = samplerate/lengthOfBin

	//treble max is just the max
	sf::Color linecolor = sf::Color::Red;

	for(float i = 3 ; i < min(bufferSize/2.f,20000.f) ; i++)
	{
		int l = abs(bin[(int)i]);
		Vector2f samplePosition(log(i)/log(min(bufferSize/2.f,20000.f)),abs(bin[(int)i]));

		if(i>bassMax)
			linecolor = sf::Color::Green;
		if (i > midMax)
			linecolor = sf::Color::Blue;
		VA2.append(Vertex(position+Vector2f(samplePosition.x*800,-samplePosition.y/max*500), linecolor));
		VA2.append(Vertex(position+Vector2f(samplePosition.x*800,0),Color::White));
		//VA2.append(Vertex(position+Vector2f(samplePosition.x*800,0),Color::Color(255,255,255,100)));
		//VA2.append(Vertex(position+Vector2f(samplePosition.x*800,samplePosition.y/max*500/2.f),Color::Color(255,255,255,0)));
	}
}
void FFT::lines(float const& max)
{
	VA3.setPrimitiveType(LineStrip) ;
	Vector2f position(0,800) ;
	Vector2f samplePosition ;
	float colorDecay = 1 ;
	
	for(float i(std::max((double)0,cascade.size()-3e5)) ; i < cascade.size() ; i++)
	{
		cascade[i].position -= Vector2f(-0.8,1) ;
		if(cascade[i].color.a != 0) cascade[i].color = Color(255,255,255,20) ;
	}
	samplePosition = Vector2f(log(3)/log(min(bufferSize/2.f,20000.f)),abs(bin[(int)3])) ;
	cascade.push_back(Vertex(position+Vector2f(samplePosition.x*800,-samplePosition.y/max*500),Color::Transparent)) ;
	for(float i(3) ; i < bufferSize/2.f ; i*=1.02)
	{
		samplePosition = Vector2f(log(i)/log(min(bufferSize/2.f,20000.f)),abs(bin[(int)i])) ;
		cascade.push_back(Vertex(position+Vector2f(samplePosition.x*800,-samplePosition.y/max*500),Color::Color(255,255,255,20))) ;
	}
	cascade.push_back(Vertex(position+Vector2f(samplePosition.x*800,-samplePosition.y/max*500),Color::Transparent)) ;

	VA3.clear() ;
	for(int i(std::max((double)0,cascade.size()-3e5)) ; i < cascade.size() ; i++) VA3.append(cascade[i]) ;
}

void FFT::draw(RenderWindow &window)
{
	window.draw(VA1) ;
	//window.draw(VA3) ;
	window.draw(VA2) ;

	window.draw(VA4);
}

float FFT::getMostCommonBPM()
{
	/*return m_bpmEstimates[ subband ];*/
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
