#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp> 
#include <iostream>
#include <fstream>
#include <sstream>
#include <deque>
#include "json.hpp"

using namespace std;
using namespace sf;
using json = nlohmann::json;

int main()
{
	RenderWindow window(VideoMode(900, 900, 32), "Window");

	string path;
	string mapPath;
	int bufferSize;
	cout << "Put an audio file in the Ressources folder (.wav will work, mp3 wont)" << endl;
	cout << "Enter the file name (example.wav) : ";
	cin >> path;	
	cout << "Enter the data file name (example.txt) : ";
	cin >> mapPath;

	ifstream audioData;
	stringstream ss;
	Music song;

	song.openFromFile(path);
	audioData.open(mapPath);
	json dat;
	ss << audioData.rdbuf();
	string s = ss.str();
	dat = json::parse(s);

	float bpm = dat["BPM"];

	vector<float> amps = dat["m"];
	deque<float> wav;
	for (int i = 0; i < 22; i++)
	{
		wav.push_back(0.f);
	}
	int lastAmp = -1;

	VertexArray waveform;
	waveform.setPrimitiveType(LineStrip);
	waveform.resize(24);
	waveform[0] = Vertex(Vector2f(20, 250), Color::Color(255, 0, 0, 255));
	waveform[23] = Vertex(Vector2f(20 +(23/ (float)24 * 700), 250), Color::Color(255, 0, 0, 255));
	for (int i = 1; i < 23; i++)
	{
		waveform[i] = Vertex(Vector2f(20+(i / (float)24 * 700), 250), Color::Color(255, 0, 0, 255));
	}

	auto cols = dat["colours"];
	vector<Color> colours;
	//for (int i = 0; i < dat["colours"].size(); i++)
	for (auto& element : dat["colours"])
	{
		colours.push_back(Color(255 / (float)element["R"], 255 / (float)element["G"], 255 / (float)element["B"]));
	}

	Clock timer;

	float beatsPerSecond = bpm / 60;
	float beatTimes = 1 / beatsPerSecond;
	float lastBeat = 0;
	RectangleShape beatSquare;
	beatSquare.setPosition(100, 400);
	beatSquare.setSize(Vector2f(100, 100.f));

	timer.restart();

	song.play();
	RectangleShape hue;
	hue.setPosition(0, 0);
	hue.setSize(Vector2f(900.f, 100.f));
	Event event;
	while (timer.getElapsedTime() < song.getDuration())
	{
		while (window.pollEvent(event)) {}
		if (song.getPlayingOffset().asSeconds() > lastBeat + beatTimes)
		{
			cout << "BEAT" << endl;
			beatSquare.setFillColor(Color::Color(255, 255, 255, 255));
			lastBeat = song.getPlayingOffset().asSeconds();
		}
		else {
			sf::Color col = beatSquare.getFillColor();

			beatSquare.setFillColor(Color::Color(255, 255, 255, col.a*((song.getPlayingOffset().asSeconds()-lastBeat - beatTimes)/beatTimes)));
		}

		float fraction = song.getPlayingOffset()/song.getDuration();
		if ((int)(fraction * 2000) != lastAmp)
		{
			lastAmp = fraction * 2000;
			wav.push_back(amps[lastAmp]);
			wav.pop_front();

			for (int i = 1; i < 23; i++)
			{
				waveform[i] = Vertex(Vector2f(20 + (i / (float)24 * 700), 250+ wav[i-1] * 0.004), Color::Color(255, 0, 0, 255));
			}
		}
		hue.setFillColor(colours[colours.size()*fraction]);

		window.clear();		
		window.draw(hue); 
		window.draw(beatSquare);
		window.draw(waveform);
		window.display();

	}

	return 0;
}

void Beat() {

}