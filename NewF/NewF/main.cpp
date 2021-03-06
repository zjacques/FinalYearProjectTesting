#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include "FFT.h"

using namespace std ;
using namespace sf ;

int main()
{
	//RenderWindow window(VideoMode(900,900,32),"Window");

	string path ;
	int bufferSize ;
	cout<<"Put an audio file in the Ressources folder (.wav will work, mp3 wont)"<<endl;
	cout<<"Enter the file name (example.wav) : " ;
	cin>>path ;
	bufferSize = 8192;

	FFT fft("Ressources/"+path,bufferSize) ;

	Event event ;

	fft.beginString();

	std::thread bpmThread = std::thread(&FFT::beatDetect, &fft);
	std::thread waveThread = std::thread(&FFT::waveForm, &fft);
	std::thread colThread = std::thread(&FFT::moodBar, &fft);
	bpmThread.join();
	waveThread.join();
	colThread.join();

	fft.endString();
	fft.printToFile();
	system("PAUSE");
	return 0;
}