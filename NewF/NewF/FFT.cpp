#include "FFT.h"

FFT::FFT(string const& _path,int const& _bufferSize)
{
	path = _path ;
	if(!buffer.loadFromFile(path)) cout<<"Unable to load buffer"<<endl ;

	sound.setBuffer(buffer) ;
	sound.setLoop(true);
	sound.play();

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
		//window.push_back(1.0f);
		//window.push_back(0.54-0.46*cos(2*PI*i/(float)bufferSize)) ;

	sample.resize(bufferSize) ;
	VA1.resize(1000);
	waveForm();
	moodBar();
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
	for (int i = 0; i < 1000; i++)
	{
		//sample[i - mark] = Complex(buffer.getSamples()[i], 0);
		float x = (float)i / (float)1000;
		float n = x * (float)sampleCount;
		Int16 sm = buffer.getSamples()[(int)n];
		VA1[i] = Vertex(Vector2f(20, 250) + Vector2f(i / (float)1000 * 700, sm*0.004), Color::Color(255, 0, 0, 255));
		//VA1[mark] = Vertex(Vector2f(20, 250) + Vector2f((mark) / (float)bufferSize * 700, sample[mark].real()*0.005), Color::Color(255, 0, 0, 50));
	}
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

		float bassMax = 250 / barBand;//the last line that should be in the bass band
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
		startSample += bufferSize/4;
	}

	//draw the actual moodbar

	//issue is: there are far more bars in the higher bands than the bass, so it will always skew blue
	for (float i = 0; i < colors.size(); i++) {
		double maxVal = std::max(colors[i].red, colors[i].green);
		maxVal = std::max(maxVal, colors[i].blue);
		colors[i].red /= maxVal;
		colors[i].green /= maxVal;
		colors[i].blue /= maxVal;
		//color should be normalized now? Skews Red now?

		VA4.append(Vertex(Vector2f((i/colors.size())*800 + 50, 400), Color(255/colors[(int)i].red, 255 / colors[(int)i].green, 255 / colors[(int)i].blue)));
		VA4.append(Vertex(Vector2f((i/colors.size()) * 800 + 50, 500), Color(255 / colors[(int)i].red, 255 / colors[(int)i].green, 255 / colors[(int)i].blue)));
	}

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
	vector<int> history;
	const float c = 1.3;
	//for each second in the song
	for (int sec = 0; sec < sampleCount; sec += sampleRate)
	{
		int second = sec / sampleRate;
		//for each window in each second
		while (currentSample < sec - (bufferSize / 2))
		{
			//hamm
			for (currentSample = startSample; currentSample < startSample + bufferSize; currentSample++)
			{
				if (currentSample < sampleCount)
				{
					sample[currentSample - startSample] = Complex(buffer.getSamples()[currentSample] * window[currentSample - startSample], 0);
				}
			}
			//hammed?

			bin2 = CArray(sample.data(), bufferSize);

			//fft
			fft(bin2);
			//fft?

			float b = bin2.size();
			float barBand = sampleRate / bin2.size();

			//grab bass

			float bassMax = 250 / barBand;//the last line that should be in the bass band
			double instantEnergy = 0.0f; //variable e in the equations

			//for each sample in window
			for (float i = 1; i < bassMax; i++)
			{
				float amplitude = abs(bin2[(int)i]);

				instantEnergy += amplitude * amplitude;

			}

			startSample += bufferSize / 4;
		}
	}

	
}

void FFT::update()
{
	hammingWindow();

	VA2.clear() ;
	//VA3.clear() ;

	bin = CArray(sample.data(),bufferSize) ;
	fft(bin) ;
	float max = 100000000 ;
	
	//lines(max) ;
	bars(max);
}

void FFT::bars(float const& max)
{
	VA2.setPrimitiveType(Lines) ;
	Vector2f position(0,800) ;
	//sampleRate
	//Bass = 0-250hz, mid = 251-5khz, treble = 5k-20k
	//width of bin bar = samplerate/lengthOfBin
	float b = bin.size();
	float barBand = sampleRate / bin.size();
	float bassMax = 250 / barBand;//the last line that should be in the bass band
	float midMax = 5000 / barBand;//the last mid line
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