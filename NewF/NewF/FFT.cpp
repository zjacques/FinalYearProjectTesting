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

	sampleRate = buffer.getSampleRate()*buffer.getChannelCount() ;
	sampleCount = buffer.getSampleCount() ;
	if(_bufferSize < sampleCount) bufferSize = _bufferSize ;
	else bufferSize = sampleCount ;
	mark = 0 ;

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

void FFT::hammingWindow()
{
	mark = sound.getPlayingOffset().asSeconds()*sampleRate;
	if (mark + bufferSize < sampleCount)
	{
		for (int i(mark); i < bufferSize + mark; i++)
		{
			sample[i - mark] = Complex(buffer.getSamples()[i] * window[i - mark], 0);
			//VA1[i - mark] = Vertex(Vector2f(20, 250) + Vector2f((i - mark) / (float)bufferSize * 700, sample[i - mark].real()*0.005), Color::Color(255, 0, 0, 50));
		}
	}
}
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

void FFT::moodBar()
{

	//for (int i = 0; i < 1024; i++)
	//{
		//hamm

		//start at 0 sample
		//get and hamm samples 0-buffersize
		//fft those and store them
		//go to 0 sample + buffersize/2
		//repeat until sampleCount

	int startSample = 0;
	int currentSample = 0;
	CArray bin2;
	vector<int> barks;
	vector<colourTriplet> colors;
	while (currentSample < sampleCount)
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
		float bassMax = 250 / barBand;//the last line that should be in the bass band
		float midMax = 5000 / barBand;//the last mid line
		colourTriplet cols;
		cols.red = 0;
		cols.green = 0;
		cols.blue = 0 ;
		for (float i = 0; i < bin2.size(); i++)
		{
			float amplitude = abs(bin2[(int)i]);

			if (i < bassMax)
				cols.red += amplitude;
			else if (i < midMax)
				cols.green += amplitude;
			else
				cols.blue += amplitude;
		}
		colors.push_back(cols);
		startSample += bufferSize;
	}

	//draw the actual moodbar
	for (float i = 0; i < colors.size(); i++) {
		double maxVal = std::max(colors[i].red, colors[i].green);
		maxVal = std::max(maxVal, colors[i].blue);
		colors[i].red /= maxVal;
		colors[i].green /= maxVal;
		colors[i].blue /= maxVal;
		//col should be normalized now?

		VA4.append(Vertex(Vector2f((i/colors.size())*800 + 50, 400), Color(255/colors[(int)i].red, 255 / colors[(int)i].green, 255 / colors[(int)i].blue)));
		VA4.append(Vertex(Vector2f((i/colors.size()) * 800 + 50, 500), Color(255 / colors[(int)i].red, 255 / colors[(int)i].green, 255 / colors[(int)i].blue)));
	}


	//sampleRate
	//Bass = 0-250hz, mid = 251-5khz, treble = 5k-20k
	//width of bin bar = samplerate/lengthOfBin
	/*float b = bin.size();
	float barBand = sampleRate / bin.size();
	float bassMax = 250 / barBand;//the last line that should be in the bass band
	float midMax = 5000 / barBand;//the last mid line
								  //treble max is just the max
	sf::Color linecolor = sf::Color::Red;

	for (float i = 3; i < min(bufferSize / 2.f, 20000.f); i++)
	{
		Vector2f samplePosition(log(i) / log(min(bufferSize / 2.f, 20000.f)), abs(bin[(int)i]));

		if (i>bassMax)
			linecolor = sf::Color::Blue;
		if (i > midMax)
			linecolor = sf::Color::Green;
	}*/


		//mark = i/1024 * sampleCount;
		/*if (mark + bufferSize < sampleCount)
		{
			for (int j = mark; j < bufferSize + mark; j++)
			{
				sample[j - mark] = Complex(buffer.getSamples()[j] * window[j - mark], 0);
				//VA1[i - mark] = Vertex(Vector2f(20, 250) + Vector2f((i - mark) / (float)bufferSize * 700, sample[i - mark].real()*0.005), Color::Color(255, 0, 0, 50));
			}
		}*/
		//fft

		//draw the bar
	//}
}

void FFT::update()
{
	hammingWindow();

	VA2.clear() ;
	VA3.clear() ;

	bin = CArray(sample.data(),bufferSize) ;
	fft(bin) ;
	float max = 100000000 ;
	
	lines(max) ;
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