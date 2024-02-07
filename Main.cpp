
#include <Siv3D.hpp>
#include <vector>
#include <cmath>

#define PI 3.141592653589
struct Location
{
	uint32 ant_id;
	uint8 caste;
	Vec2 pos;
	double body_rot;
	Color color;
};


class Settings {
private:
public:
	JSON settings_json;

	uint32 window_width;
	uint32 window_height;

	uint32 video_width;
	uint32 video_height;

	uint32 video_fps;
	double time_stamp_range;
	double delta_sec;

	Settings(String _settings_file)
	{
		settings_json = JSON::Load(_settings_file);
		window_width = settings_json[U"windows"][U"size_width"].get<uint32>();
		window_height = settings_json[U"windows"][U"size_height"].get<uint32>();

		video_width = settings_json[U"videos"][U"size_width"].get<uint32>();
		video_height = settings_json[U"videos"][U"size_width"].get<uint32>();
		video_fps = settings_json[U"videos"][U"fps"].get<uint32>();
		time_stamp_range = settings_json[U"videos"][U"time_stamp_range"].get<double>();
		delta_sec = settings_json[U"videos"][U"delta_sec"].get<double>();
	}
};

class Termite
{
private:

	uint32 window_width = 1280;
	uint32 window_height = 720;

	uint32 ant_id;
	uint8 caste;

	Ellipse head;
	Vec2 head_pos;
	int16 head_height = 7;
	int16 head_width = 7;

	Ellipse body;
	Vec2 body_pos;
	double body_rot = 1.0;
	int16 body_height = 5;
	int16 body_width = 5;
	int16 body_length = 40;

	Color m_color;

	// 最後につかまれた時刻(ミリ秒)
	uint32 m_lastUpdateTimeSec = 0;
	//double PI = 3.141592653589;

	bool body_grabbed = false;
	bool head_grabbed = false;

public:

	Termite(Vec2 _head_pos, int16 _body_rot, Color _m_color)
	{
		ant_id = 0;
		caste = 0;
		head_pos = _head_pos;
		head = Ellipse(head_pos, head_width, head_height);

		body_rot = _body_rot;
		body_pos = Vec2{ head_pos.x + body_length * cos(body_rot), head_pos.y + body_length * sin(body_rot) };
		body = Ellipse(body_pos, body_width, body_height);

		m_color = Color(_m_color.r, _m_color.g, _m_color.b);
	}
	Termite(Location location)
	{
		ant_id = location.ant_id;
		caste = location.caste;
		head_pos = location.pos;
		head = Ellipse(head_pos, head_width, head_height);

		body_rot = location.body_rot;
		body_pos = Vec2{ head_pos.x + body_length * cos(body_rot), head_pos.y + body_length * sin(body_rot) };
		body = Ellipse(body_pos, body_width, body_height);

		m_color = Color(location.color.r, location.color.g, location.color.b);
	}

	const Ellipse& getHead() const
	{
		return head;
	}
	const Ellipse& getBody() const
	{
		return body;
	}

	String update()
	{
		if (head.leftClicked())
		{
			head_grabbed = true;
			m_lastUpdateTimeSec = Time::GetMillisec();

			return U"headLeftClicked";
		}
		else if (head_grabbed)
		{
			if (MouseL.up())
			{
				head_grabbed = false;
				body_grabbed = false;
				return U"MouseLUp";
			}
			head.moveBy(Cursor::Delta());
			if (head.center.x > window_width) {
				head.setPos(Vec2(window_width, head.center.y));
			}
			else if (head.center.x < 0) {
				head.setPos(Vec2(0, head.center.y));
			}
			if (head.center.y > window_height) {
				head.setPos(Vec2(head.center.x, window_height));
			}
			else if (head.center.y < 0) {
				head.setPos(Vec2(head.center.x, 0));
			}
			head_pos = head.center;
			//body.moveBy(Cursor::Delta());
			body_pos = Vec2{ head_pos.x + body_length * cos(body_rot), head_pos.y + body_length * sin(body_rot) };
			body.setPos(body_pos);
			if (KeyA.pressed()) {
				body_rot = body_rot + 0.02;
				body_pos = Vec2{ head_pos.x + body_length * cos(body_rot), head_pos.y + body_length * sin(body_rot) };
				body = Ellipse(body_pos, body_width, body_height);
			}
			else if (KeyD.pressed()) {
				body_rot = body_rot - 0.02;
				body_pos = Vec2{ head_pos.x + body_length * cos(body_rot), head_pos.y + body_length * sin(body_rot) };
				body = Ellipse(body_pos, body_width, body_height);

			}
			return U"bodyGrabbed";
		}

		if (body.leftClicked())
		{
			body_grabbed = true;
			m_lastUpdateTimeSec = Time::GetMillisec();
			return U"bodyLeftClicked";
		}
		else if (body_grabbed)
		{
			if (MouseL.up())
			{
				head_grabbed = false;
				body_grabbed = false;
				return U"MouseLUp";
			}
			body_rot = (atan2(head_pos.y - Cursor::PosF().y, head_pos.x - Cursor::PosF().x) + PI);
			body_pos = Vec2{ head_pos.x + body_length * cos(body_rot), head_pos.y + body_length * sin(body_rot) };
			body = Ellipse(body_pos, body_width, body_height);
			return U"bodyGrabbed";
		}
		return U"noUpdate";
	}

	void draw(uint8 alpha) const
	{
		Color draw_color = Color(m_color.r, m_color.g, m_color.b, alpha);
		Line{ head_pos, body_pos }.draw(LineStyle::SquareDot, 3, draw_color);
		head.draw(draw_color);
		body.draw(draw_color);

		if (caste == 1) {
			Shape2D::Star(5, head_pos).draw(Palette::Yellow);
		}
	}

	uint32 getLastUpdateTime() const
	{
		return m_lastUpdateTimeSec;
	}

	Location getLocation() const
	{
		Location location;
		location.ant_id = ant_id;
		location.caste = caste;
		location.pos = head_pos;
		location.color = m_color;
		location.body_rot = body_rot;

		return location;
	}

	String update_by_json(Location location) {
		ant_id = location.ant_id;
		caste = location.caste;
		head_pos = location.pos;
		m_color = location.color;
		body_rot = location.body_rot;

		head = Ellipse(head_pos, head_width, head_height);
		body_pos = Vec2{ head_pos.x + body_length * cos(body_rot), head_pos.y + body_length * sin(body_rot) };
		body = Ellipse(body_pos, body_width, body_height);
		return U"UpdateByLocation";
	}
	Color get_color() {
		return m_color;
	}
	String set_color(Color color) {
		m_color = color;
		return U"Success";
	}
};

class TermiteVideo
{
private:
	VideoTexture videoTexture;
	bool is_play = false;
	double time_range = 1;
	//double step_sec = 0.005;
	double video_pos_sec;
	double video_length_sec;

public:

	TermiteVideo(String video_path)
	{
		videoTexture = VideoTexture{ video_path, Loop::Yes };
		video_pos_sec = videoTexture.posSec();
		video_length_sec = videoTexture.lengthSec();

	}

	uint32 now_time_stamp() {
		uint32 now_time_stamp = static_cast<uint32>(floor(videoTexture.posSec() / time_range));
		return now_time_stamp;
	}
	//String advance_video(double advance_sec)
	//{
	//	if (video_pos_sec + advance_sec > video_length_sec) {
	//		return U"ErrorOutOfRange";
	//	}
	//	else if (video_pos_sec + advance_sec < 0) {
	//		return U"ErrorOutOfRange";
	//	}
	//	else if (video_pos_sec + advance_sec <= video_length_sec) {
	//		//video_pos_sec = video_pos_sec + advance_sec;
	//		//videoTexture.setPosSec(video_pos_sec);
	//		videoTexture.advance(advance_sec);
	//		video_pos_sec = videoTexture.posSec();
	//		return U"Success";
	//	}
	//}
	String set_video_sec(double pos_sec) {
		if (pos_sec > video_length_sec) {
			return U"ErrorOutOfRange";
		}
		else if (pos_sec < 0) {
			return U"ErrorOutOfRange";
		}
		else {
			videoTexture.setPosSec(pos_sec);
			video_pos_sec = videoTexture.posSec();
			return U"Success";
		}
	}
	String set_video_time_stamp(uint32 time_stamp) {
		return set_video_sec(time_stamp * time_range);
	}

	void reverse_play()
	{
		is_play = !is_play;
	}

	void set_is_play(bool _is_play) {
		is_play = _is_play;
	}

	bool get_is_play()
	{
		return is_play;
	}

	void draw()
	{
		videoTexture.advance(0);
		videoTexture.draw();
	}
	String update()
	{
		video_pos_sec = videoTexture.posSec();
		uint32 old_time_stamp = now_time_stamp();
		bool change_video_pos_sec = SimpleGUI::Slider(U"{:.5f}/{:.5f}"_fmt(video_pos_sec, video_length_sec), video_pos_sec, 0.0, video_length_sec, Vec2{ 720, 0 }, 130, 400);
		Print << videoTexture.posSec();
		//videoTexture.setPosSec
		if (change_video_pos_sec)
		{
			is_play = false;
			videoTexture.setPosSec(video_pos_sec);
			//Print << videoTexture.posSec();
			if (now_time_stamp() != old_time_stamp) {
				return U"ChangeTimeStamp";
			}
			return U"ChangeVideoPosSecSlider";
		}
		else if (is_play) {
			videoTexture.advance();
			if (now_time_stamp() > old_time_stamp) {
				is_play = false;
				return U"ChangeTimeStamp";
				//String set_video_message = set_video_time_stamp(now_time_stamp());
				//if (set_video_message == U"ErrorOutOfRange") {
				//	is_play = false;
				//	return U"ErrorOutOfRange";
				//}
				//else if (set_video_message == U"Success")
				//{
				//	return U"ChangeTimeStamp";
				//}
			}

		}
		return U"noUpdate";

	}
};

// jsonとlocationのエンドポイント
class JsonLocationMiddlwear
{
private:
	JSON locations_json;
	String locations_file;
	//Array<Location> locations;

public:
	JsonLocationMiddlwear(String _locations_file)
	{
		locations_file = _locations_file;
		locations_json = JSON::Load(_locations_file);
	}

	Array<Location> read_locations(uint32 time_stamp)
	{
		JSON json = locations_json[U"locations"][U"{}"_fmt(time_stamp)];
		if (json.isNull()) { throw Error(U"値が定義されていません"); }
		else {
			Array<Location> locations;
			for (const JSON location_json : json.arrayView())
			{
				Location location;
				location.ant_id = location_json[U"ant_id"].get<int32>();
				JSON termite_info = locations_json[U"termites"][U"{}"_fmt(location.ant_id)];
				location.caste = termite_info[U"caste"].get<int8>();
				location.pos = Vec2(location_json[U"pos_x"].get<int32>(), location_json[U"pos_y"].get<int32>());
				location.body_rot = location_json[U"body_rot"].get<double>();
				location.color = Color(termite_info[U"color"][U"r"].get<uint8>(), termite_info[U"color"][U"g"].get<uint8>(), termite_info[U"color"][U"b"].get<uint8>());
				locations << location;
			}
			return locations;
		}
	}

	String save_json(uint32 time_stamp, Array<Location> locations) {
		Array<JSON> new_locations_array;
		//Array colors;

		for (const Location& location : locations)
		{
			JSON new_json;
			new_json[U"ant_id"] = location.ant_id;
			new_json[U"pos_x"] = location.pos.x;
			new_json[U"pos_y"] = location.pos.y;
			new_json[U"body_rot"] = location.body_rot;
			locations_json[U"termites"][U"{}"_fmt(location.ant_id)][U"color"][U"r"] = location.color.r;
			locations_json[U"termites"][U"{}"_fmt(location.ant_id)][U"color"][U"g"] = location.color.g;
			locations_json[U"termites"][U"{}"_fmt(location.ant_id)][U"color"][U"b"] = location.color.b;
			locations_json[U"termites"][U"{}"_fmt(location.ant_id)][U"caste"] = location.caste;
			//Print << new_json;
			new_locations_array << new_json;
		}
		locations_json[U"locations"][U"{}"_fmt(time_stamp)] = new_locations_array;
		locations_json.save(locations_file);
		return U"Success";
	}

	String save_json_termite(uint32 time_stamp, Array<Termite> termites) {
		Array<JSON> new_json_array;
		for (const Termite& termite : termites)
		{
			Location location = termite.getLocation();
			JSON new_json;
			new_json[U"ant_id"] = location.ant_id;
			new_json[U"pos_x"] = location.pos.x;
			new_json[U"pos_y"] = location.pos.y;
			new_json[U"body_rot"] = location.body_rot;
			locations_json[U"termites"][U"{}"_fmt(location.ant_id)][U"color"][U"r"] = location.color.r;
			locations_json[U"termites"][U"{}"_fmt(location.ant_id)][U"color"][U"g"] = location.color.g;
			locations_json[U"termites"][U"{}"_fmt(location.ant_id)][U"color"][U"b"] = location.color.b;
			locations_json[U"termites"][U"{}"_fmt(location.ant_id)][U"caste"] = location.caste;
			new_json_array << new_json;
			//Console << new_json;
		}
		//Console << JSON(new_json_array);
		locations_json[U"locations"][U"{}"_fmt(time_stamp)] = JSON(new_json_array);
		//Console << new_json_array;
		locations_json.save(locations_file);
		return U"Success";
	}

	String json_state(uint32 time_stamp)
	{
		JSON json = locations_json[U"locations"][U"{}"_fmt(time_stamp)];
		if (json.isNull())
		{
			return U"isNull";
		}
		else {
			return U"Empty";
		}

	}
};

struct UpdateTimeIndex {
	uint32 index;
	uint32 last_update_time;
};

Array<UpdateTimeIndex> sort_termite_draw_order(Array<Termite> termites) {

	Array<UpdateTimeIndex> termite_draw_indexes;

	for (uint32 i = 0; i < termites.size(); i++)
	{
		UpdateTimeIndex last_update_time_index;
		last_update_time_index.index = i;
		last_update_time_index.last_update_time = termites[i].getLastUpdateTime();
		termite_draw_indexes << last_update_time_index;
	}
	std::sort(termite_draw_indexes.begin(), termite_draw_indexes.end(), [](const UpdateTimeIndex& a, const UpdateTimeIndex& b) {
		return b.last_update_time < a.last_update_time;
	});

	return termite_draw_indexes;
}

void Main()
{
	Window::Resize(1280, 720);
	Scene::SetBackground(Palette::White);

	TermiteVideo termite_video = TermiteVideo(U"termite_video.mp4");
	JsonLocationMiddlwear location_middlwear = JsonLocationMiddlwear(U"locations.json");


	Array<Location> locations = location_middlwear.read_locations(0);
	Array<Termite> termites;
	// シロアリ配列の格納
	for (uint32 i = 0; i < locations.size(); i++)
	{
		Termite termite = Termite{ locations[i] };
		termites << termite;
	}
	Array<UpdateTimeIndex> termite_draw_order = sort_termite_draw_order(termites);

	int32 now_time_stamp = 0;

	bool handCursor = false;

	while (System::Update())
	{
		ClearPrint();

		//SimpleGUI::ColorPicker(color0, Vec2{ 100, 100 })
		HSV m_hsv = HSV(termites[termite_draw_order[0].index].get_color());
		bool update_hsv = SimpleGUI::ColorPicker(m_hsv, Vec2{ 720, 120 });
		if (update_hsv) {
			termites[termite_draw_order[0].index].set_color(Color(m_hsv));
		}
		Cursor::RequestStyle(handCursor ? CursorStyle::Hand : CursorStyle::Default);

		if (SimpleGUI::Button(U"\U000F054C コピー", Vec2{ 720, 80 }))
		{
			String json_state = location_middlwear.json_state(termite_video.now_time_stamp() - 1);
			if (json_state == U"Empty") {
				locations = location_middlwear.read_locations(termite_video.now_time_stamp() - 1);
				termites.clear();
				// シロアリ配列の格納
				for (uint32 i = 0; i < locations.size(); i++)
				{
					Termite termite = Termite{ locations[i] };
					termites << termite;
				}
				termite_draw_order = sort_termite_draw_order(termites);
			}
		}
		else if (SimpleGUI::Button(U"\U000F054C", Vec2{ 720, 40 }, 40)) {
			String message = termite_video.set_video_time_stamp(termite_video.now_time_stamp() - 1);

			if (message == U"Success") {
				String json_state = location_middlwear.json_state(termite_video.now_time_stamp());
				if (json_state == U"Empty") {
					locations = location_middlwear.read_locations(termite_video.now_time_stamp());
					termites.clear();
					// シロアリ配列の格納
					for (uint32 i = 0; i < locations.size(); i++)
					{
						Termite termite = Termite{ locations[i] };
						termites << termite;
					}
					termite_draw_order = sort_termite_draw_order(termites);
				}
				else if (json_state == U"isNull") {
					//Console << U"error";
					location_middlwear.save_json_termite(termite_video.now_time_stamp(), termites);
				}
			}
		}
		else if (SimpleGUI::Button(U"\U000F044E", Vec2{ 770, 40 }, 40)) {
			String message = termite_video.set_video_time_stamp(termite_video.now_time_stamp() + 1);
			if (message == U"Success") {
				String json_state = location_middlwear.json_state(termite_video.now_time_stamp());
				if (json_state == U"Empty") {
					locations = location_middlwear.read_locations(termite_video.now_time_stamp());
					termites.clear();
					// シロアリ配列の格納
					for (uint32 i = 0; i < locations.size(); i++)
					{
						Termite termite = Termite{ locations[i] };
						termites << termite;
					}
					termite_draw_order = sort_termite_draw_order(termites);
				}
				else if (json_state == U"isNull") {
					//Console << U"error";
					location_middlwear.save_json_termite(termite_video.now_time_stamp(), termites);
				}
			}
		}

		now_time_stamp = termite_video.now_time_stamp();
		termite_draw_order = sort_termite_draw_order(termites);

		String termite_video_update = termite_video.update();

		if (termite_video_update == U"ChangeTimeStamp") {
			String message = termite_video.set_video_time_stamp(termite_video.now_time_stamp());

			if (message == U"Success") {
				String json_state = location_middlwear.json_state(termite_video.now_time_stamp());
				if (json_state == U"Empty") {
					locations = location_middlwear.read_locations(termite_video.now_time_stamp());
					termites.clear();
					// シロアリ配列の格納
					for (uint32 i = 0; i < locations.size(); i++)
					{
						Termite termite = Termite{ locations[i] };
						termites << termite;
					}
					termite_draw_order = sort_termite_draw_order(termites);
				}
				else if (json_state == U"isNull") {
					//Console << U"error";
					location_middlwear.save_json_termite(termite_video.now_time_stamp(), termites);
				}
			}
			//String json_state = location_middlwear.json_state(termite_video.now_time_stamp());
			//if (json_state == U"Empty") {
			//	locations = location_middlwear.read_locations(termite_video.now_time_stamp());
			//	termites.clear();
			//	// シロアリ配列の格納
			//	for (uint32 i = 0; i < locations.size(); i++)
			//	{
			//		Termite termite = Termite{ locations[i] };
			//		termites << termite;
			//	}
			//	termite_draw_order = sort_termite_draw_order(termites);
			//}
			//else if (json_state == U"isNull") {
			//	//Console << U"error";
			//	location_middlwear.save_json_termite(termite_video.now_time_stamp(), termites);
			//}
		}

		termite_video.draw();


		for (const UpdateTimeIndex& termite_indexe : termite_draw_order)
		{
			// 1 つの円がつかまれたら、それ以外の円は処理しない
			String termite_update = termites[termite_indexe.index].update();
			//Print << termite_update;
			//Print << termite_update;
			if (termite_update == U"MouseLUp")
			{
				location_middlwear.save_json_termite(now_time_stamp, termites);
				break;
			}
			else if (termite_update != U"noUpdate")
			{
				termite_video.set_is_play(false);
				break;
			}
			else if (termite_update == U"noUpdate") {
				/*if (KeySpace.down())
				{
					termite_video.reverse_play();
				}*/
			}
		}

		uint32 order = 0;
		for (const UpdateTimeIndex& termite_indexe : termite_draw_order)
		{
			if (order == 0) {
				termites[termite_indexe.index].draw(150);
			}
			else {
				termites[termite_indexe.index].draw(80);
			}
			order = order + 1;
		}

		if (KeySpace.down())
		{
			termite_video.reverse_play();
		}
		handCursor = false;
		for (const Termite& termite : termites)
		{
			if (termite.getHead().mouseOver())
			{
				handCursor = true;
				break;
			}
			if (termite.getBody().mouseOver())
			{
				handCursor = true;
				break;
			}
		}
		Print << termite_video.now_time_stamp();
	}
}
