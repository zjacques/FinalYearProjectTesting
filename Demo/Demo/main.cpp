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
	cout << "Put an audio file in the Ressources folder (.wav will work, mp3 wont)" << endl;
	cout << "Enter the file name (example.wav) : ";
	cin >> path;	
	cout << "Enter the data file name (example.txt) : ";
	cin >> mapPath;

	ifstream audioData;
	stringstream ss;
	Music song;

	song.openFromFile("Ressources/" + path);
	audioData.open(mapPath);
	json dat;
	ss << audioData.rdbuf();
	string s = ss.str();
	dat = json::parse(s);

	float bpm = dat["BPM"];

	vector<float> amps = dat["m"];
	deque<float> wav;
	for (int i = 0; i < 50; i++)
	{
		wav.push_back(0.f);
	}
	int lastAmp = -1;

	int numpoints = amps.size();
	VertexArray waveform;
	waveform.setPrimitiveType(LineStrip);
	waveform.resize(50);
	waveform[0] = Vertex(Vector2f(20, 250), Color::Color(255, 0, 0, 255));
	waveform[49] = Vertex(Vector2f(20 +(49 / (float)50 * 880), 250), Color::Color(255, 0, 0, 255));
	for (int i = 1; i < 49; i++)
	{
		waveform[i] = Vertex(Vector2f(20+(i / (float)50 * 880), 250), Color::Color(255, 0, 0, 255));
	}

	auto cols = dat["colours"];
	vector<Color> colours;
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

	//colour bars
	RectangleShape redBar;
	RectangleShape greenBar;
	RectangleShape blueBar;
	redBar.setPosition(0,900);
	redBar.setSize(Vector2f(300.f, 00.f));
	redBar.setFillColor(Color::Red);
	greenBar.setPosition(300, 900);
	greenBar.setSize(Vector2f(300.f, 00.f));
	greenBar.setFillColor(Color::Green);
	blueBar.setPosition(600, 900);
	blueBar.setSize(Vector2f(300.f, 00.f));
	blueBar.setFillColor(Color::Blue);


	//begin replay
	timer.restart();

	song.play();
	RectangleShape hue;
	hue.setPosition(0, 0);
	hue.setSize(Vector2f(900.f, 100.f));
	Event event;
	while (timer.getElapsedTime() < song.getDuration())
	{
		while (window.pollEvent(event)) {}
		float playoffset = song.getPlayingOffset().asSeconds();
		if (playoffset >= lastBeat + beatTimes)
		{
			beatSquare.setFillColor(Color::Color(255, 255, 255, 255));
			lastBeat = playoffset;
		}
		else {
			//sf::Color col = beatSquare.getFillColor();
			float frac = ((playoffset - lastBeat)/beatTimes);
			if (frac > 1) frac = 1;
			beatSquare.setFillColor(Color::Color(255, 255, 255, 255*(1-frac)));
		}

		float fraction = song.getPlayingOffset()/song.getDuration();
		if (fraction > 1) fraction = 1;
		if ((int)(fraction * numpoints) != lastAmp)
		{
			lastAmp = fraction * numpoints;
			wav.push_back(amps[lastAmp]);
			wav.pop_front();

			for (int i = 1; i < 49; i++)
			{
				waveform[i] = Vertex(Vector2f(20 + (i / (float)50 * 880), 250+ wav[i-1] * 0.004), Color::Color(255, 0, 0, 255));
			}
		}
		hue.setFillColor(colours[colours.size()*fraction]);
		redBar.setSize(Vector2f(300, -colours[colours.size()*fraction].r));
		greenBar.setSize(Vector2f(300, -colours[colours.size()*fraction].g));
		blueBar.setSize(Vector2f(300, -colours[colours.size()*fraction].b));

		window.clear();
		window.draw(hue); 
		window.draw(beatSquare);
		window.draw(waveform);
		window.draw(redBar);
		window.draw(greenBar);
		window.draw(blueBar);
		window.display();

	}

	return 0;
}