#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp> 
#include <iostream>
#include <fstream>
#include <sstream>
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
	auto cols = dat["colours"];
	vector<Color> colours;
	//for (int i = 0; i < dat["colours"].size(); i++)
	for (auto& element : dat["colours"])
	{
		colours.push_back(Color(255 / (float)element["R"], 255 / (float)element["G"], 255 / (float)element["B"]));
	}

	Clock timer;

	timer.restart();

	song.play();
	RectangleShape hue;
	hue.setPosition(0, 0);
	hue.setSize(Vector2f(900.f, 100.f));
	Event event;
	while (timer.getElapsedTime() < song.getDuration())
	{
		while (window.pollEvent(event)) {}
		float fraction = song.getPlayingOffset()/song.getDuration();

		hue.setFillColor(colours[colours.size()*fraction]);

		window.clear();
		window.draw(hue);
		window.display();

	}

	return 0;
}

void Beat() {

}