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

int wavelength = 32;

int main()
{
	RenderWindow window(VideoMode(1920, 1080, 32), "Window");

	string path;
	string mapPath;
	cout << "Put an audio file in the Ressources folder (.wav will work, mp3 wont)" << endl;
	cout << "Enter the file name (example.wav) : ";
	cin >> path;

	mapPath = path.substr(0, path.find("."));
	mapPath += ".txt";

	sf::Texture bgTex;
	if (!bgTex.loadFromFile("Ressources/1.png"))
	{
		// error...
	}
	bgTex.setSmooth(true);
	Sprite bgSprite;
	bgSprite.setTexture(bgTex);

	sf::Texture sunTex;
	if (!sunTex.loadFromFile("Ressources/sun.png"))
	{
		// error...
	}
	sunTex.setSmooth(true);
	Sprite sunSprite;
	sunSprite.setTexture(sunTex);	
	
	sf::Texture fgTex;
	if (!fgTex.loadFromFile("Ressources/fg.png"))
	{
		// error...
	}
	fgTex.setSmooth(true);
	Sprite fgSprite;
	fgSprite.setTexture(fgTex);

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
	for (int i = 0; i < wavelength; i++)
	{
		wav.push_back(0.f);
	}
	int lastAmp = -1;

	int numpoints = amps.size();
	VertexArray waveform;
	waveform.setPrimitiveType(TriangleStrip);
	waveform.resize(wavelength*2);
	waveform[0] = Vertex(Vector2f(1 / (float)wavelength * 960, 685), Color::Color(255, 0, 255, 255));
	waveform[1] = Vertex(Vector2f(1 / (float)wavelength * 960, 690), Color::Color(255, 0, 255, 255));
	for (int i = 2; i < wavelength*2; i+=2)
	{
		waveform[i] = Vertex(Vector2f(0 + (i/2 / (float)wavelength * 960), 685), Color::Color(255, 0, 255, 255));
		waveform[i+1] = Vertex(Vector2f(0 + (i / (float)wavelength * 960), 690), Color::Color(255, 0, 255, 255));
	}

	VertexArray waveform2;
	waveform2.setPrimitiveType(TriangleStrip);
	waveform2.resize(wavelength * 2);
	waveform2[0] = Vertex(Vector2f(1920- 1 / (float)wavelength * 960, 685), Color::Color(255, 0, 255, 255));
	waveform2[1] = Vertex(Vector2f(1920- 1 / (float)wavelength * 960, 690), Color::Color(255, 0, 255, 255));
	for (int i = 2; i < wavelength * 2; i += 2)
	{
		waveform2[i] = Vertex(Vector2f(1920 - (i/2 / (float)wavelength * 960), 685), Color::Color(255, 0, 255, 255));
		waveform2[i + 1] = Vertex(Vector2f(1920 - (i/2 / (float)wavelength * 960), 690), Color::Color(255, 0, 255, 255));
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

	//begin replay
	timer.restart();

	song.play();
	Event event;
	while (timer.getElapsedTime() < song.getDuration())
	{
		while (window.pollEvent(event)) {}
		float playoffset = song.getPlayingOffset().asSeconds();
		if (playoffset >= lastBeat + beatTimes)
		{
			sunSprite.setColor(Color::Color(255, 255, 255, 255));
			lastBeat = playoffset;
		}
		else {
			float frac = ((playoffset - lastBeat) / beatTimes);
			if (frac > 1) frac = 1;
			sunSprite.setColor(Color::Color(255, 255, 255, 255 * max((1 - frac),0.25f)));
		}

		float fraction = song.getPlayingOffset() / song.getDuration();
		if (fraction > 1) fraction = 1;
		if ((int)(fraction * numpoints) != lastAmp)
		{
			lastAmp = fraction * numpoints;
			wav.push_back(amps[lastAmp]);
			wav.pop_front();

			for (int i = 2; i < wavelength*2; i+=2)
			{
				waveform[i] = Vertex(Vector2f(0 + ((i+2)/2 / (float)wavelength * 960), 685 + wav[i/2 - 1] * 0.008), Color::Color(255, 0, 255, 255));
				waveform[i+1] = Vertex(Vector2f(0 + ((i + 2) /2 / (float)wavelength * 960), 690 + wav[i / 2 - 1] * 0.008), Color::Color(255, 0, 255, 255));

				waveform2[i] = Vertex(Vector2f(1920 - ((i + 2) / 2 / (float)wavelength * 960), 685 + wav[i / 2 - 1] * 0.008), Color::Color(255, 0, 255, 255));
				waveform2[i + 1] = Vertex(Vector2f(1920 - ((i + 2) / 2 / (float)wavelength * 960), 690 + wav[i / 2 - 1] * 0.008), Color::Color(255, 0, 255, 255));

			}

		}
		bgSprite.setColor(colours[colours.size()*fraction]);

		window.clear();
		window.draw(bgSprite);
		window.draw(sunSprite);
		window.draw(fgSprite);

		window.draw(waveform);
		window.draw(waveform2);
		window.display();

	}

	return 0;
}