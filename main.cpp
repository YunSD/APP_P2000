/*
 * This source file is part of RmlUi, the HTML/CSS Interface Middleware
 *
 * For the latest information, see http://github.com/mikke89/RmlUi
 *
 * Copyright (c) 2018 Michael R. P. Ragazzon
 * Copyright (c) 2019 The RmlUi Team, and contributors
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include <RmlUi/Core.h>
#include <RmlUi/Core/StreamMemory.h>
#include <RmlUi/Core/TransformPrimitive.h>
#include <RmlUi/Debugger.h>
#include "backends/RmlUi_Backend.h"
#include "shell/include/Shell.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include "log.hpp"
#include "sensor.hpp"
#include <codecvt>
#include "csv_parse.h"
#include "model_automation.hpp"

static const Rml::String sandbox_default_rcss = R"(
body { top: 0; left: 0; right: 0; bottom: 0; overflow: hidden auto; }
scrollbarvertical { width: 15px; }
scrollbarvertical slidertrack { background: #eee; }
scrollbarvertical slidertrack:active { background: #ddd; }
scrollbarvertical sliderbar { width: 15px; min-height: 30px; background: #aaa; }
scrollbarvertical sliderbar:hover { background: #888; }
scrollbarvertical sliderbar:active { background: #666; }
scrollbarhorizontal { height: 15px; }
scrollbarhorizontal slidertrack { background: #eee; }
scrollbarhorizontal slidertrack:active { background: #ddd; }
scrollbarhorizontal sliderbar { height: 15px; min-width: 30px; background: #aaa; }
scrollbarhorizontal sliderbar:hover { background: #888; }
scrollbarhorizontal sliderbar:active { background: #666; }
)";

// 全局变量池：启动时进行扫描
static std::vector<sensor*> sensor_list;

const static int parse_sensor() {
	const static std::string filename = "sensor.csv";
	try {
		io::CSVReader<2> in(filename);
		in.set_header("index", "host");
		int index;
		std::string host;
		int i = 1;
		while (in.read_row(index, host)) {
			// do stuff with the data
			sensor* item = new sensor(index, host);
			sensor_list.push_back(item);
		}
		return 1;
	}
	catch (const char* error) {
		Log::LOG_ERROR(error);
	}
	catch (...) {
		Log::LOG_ERROR("Unknown Error:sensor config parse!");
	}
	return 0;
}

const static std::string p_green = "div-li-green";
const static std::string p_red = "div-li-red";
const static std::string block_red = "div-li-red-block";
const static std::string block_green = "div-li-green-block";


class DemoWindow : public Rml::EventListener
{
public:
	DemoWindow(const Rml::String& title, Rml::Context* context):mode_a(sensor_list)
	{
		using namespace Rml;
		document = context->LoadDocument("demo.rml");
		if (document)
		{
			//document->GetElementById("title")->SetInnerRML(title);
			//document->GetElementById("").
			if (auto tabset = static_cast<Rml::ElementTabSet*>(document->GetElementById("menu"))) {
				// 如果是生产模式，禁止进行配置
				//tabset->RemoveTab(1);
			}
			if (auto target = document->GetElementById("monitor_panel") ) {
				
				if(auto select_source = static_cast<Rml::ElementFormControlSelect*>(document->GetElementById("select_sensor_list"))){
					// 初始化界面
					std::stringstream ss;
					for (std::vector<sensor*>::iterator it = sensor_list.begin(); it != sensor_list.end(); ++it) {
						ss.str("");
						ss << "sensor-div" << (*it)->index;
						Rml::ElementPtr div_ptr = document->CreateElement("div");
						div_ptr->SetClassNames(p_green);
						div_ptr->SetId(ss.str());

						ss.str("");
						ss << "sensor-block" << (*it)->index;
						Rml::ElementPtr div_block_ptr = document->CreateElement("div");
						div_block_ptr->SetClassNames(block_red);
						div_block_ptr->SetId(ss.str());

						Rml::ElementPtr div_block_p_ptr = document->CreateElement("div");
						div_block_p_ptr->SetClassNames("div-li-p");

						ss.str("");
						ss << u8"传感器-" << (*it)->index << u8"：" << (*it)->host;
						Rml::ElementPtr p_ptr = document->CreateElement("p");
						p_ptr->SetInnerRML(ss.str());

						div_block_p_ptr->AppendChild(std::move(p_ptr));
						div_ptr->AppendChild(std::move(div_block_ptr));
						div_ptr->AppendChild(std::move(div_block_p_ptr));

						target->AppendChild(std::move(div_ptr));

						// 初始化
						ss.str("");
						ss << u8"传感器 - " << (*it)->index << u8"：" << (*it)->host;
						select_source->Add(ss.str(), std::to_string((*it)->index));
					}
				}
			}
			

			// Add sandbox default text.
			if (auto source = static_cast<Rml::ElementFormControl*>(document->GetElementById("sandbox_rml_source")))
			{
				auto value = source->GetValue();
				value += "<p>Write your RML here</p>\n\n<!-- <img src=\"assets/high_scores_alien_1.tga\"/> -->";
				source->SetValue(value);
			}

			// Prepare sandbox document.
			if (auto target = document->GetElementById("sandbox_target"))
			{
				iframe = context->CreateDocument();
				auto iframe_ptr = iframe->GetParentNode()->RemoveChild(iframe);
				target->AppendChild(std::move(iframe_ptr));
				iframe->SetProperty(PropertyId::Position, Property(Style::Position::Absolute));
				iframe->SetProperty(PropertyId::Display, Property(Style::Display::Block));
				iframe->SetInnerRML("<p>Rendered output goes here.</p>");

				// Load basic RML style sheet
				Rml::String style_sheet_content;
				{
					// Load file into string
					auto file_interface = Rml::GetFileInterface();
					Rml::FileHandle handle = file_interface->Open("assets/rml.rcss");

					size_t length = file_interface->Length(handle);
					style_sheet_content.resize(length);
					file_interface->Read((void*)style_sheet_content.data(), length, handle);
					file_interface->Close(handle);

					style_sheet_content += sandbox_default_rcss;
				}

				Rml::StreamMemory stream((Rml::byte*)style_sheet_content.data(), style_sheet_content.size());
				stream.SetSourceURL("sandbox://default_rcss");

				rml_basic_style_sheet = MakeShared<Rml::StyleSheetContainer>();
				rml_basic_style_sheet->LoadStyleSheetContainer(&stream);
			}

			// Add sandbox style sheet text.
			if (auto source = static_cast<Rml::ElementFormControl*>(document->GetElementById("sandbox_rcss_source")))
			{
				Rml::String value = "/* Write your RCSS here */\n\n/* body { color: #fea; background: #224; }\nimg { image-color: red; } */";
				source->SetValue(value);
				SetSandboxStylesheet(value);
			}

			gauge = document->GetElementById("gauge");
			progress_horizontal = document->GetElementById("progress_horizontal");

			document->Show();
		}
		mode_a.start();
	}

	void Update() {
		if (iframe)
		{
			// 应该是用于 css 以及 效果更新
			iframe->UpdateDocument();
			// 刷新传感器状态
			static std::stringstream ss;
			for (std::vector<sensor*>::iterator it = sensor_list.begin(); it != sensor_list.end(); ++it) {

				ss.str("");
				ss << "sensor-div" << (*it)->index;
				if (auto target = document->GetElementById(ss.str())) {
					if ((*it)->getConnectFlag()) {
						target->SetClassNames(p_green);
					}
					else {
						target->SetClassNames(p_red);
					}
				}

				ss.str("");
				ss << "sensor-block" << (*it)->index;
				if (auto target = document->GetElementById(ss.str())) {
					if ((*it)->getConnectFlag()) {
						target->SetClassNames(block_green);
					}
					else {
						target->SetClassNames(block_red);
					}
				}
			}

		}
		if (submitting && gauge && progress_horizontal)
		{
			using namespace Rml;
			constexpr float progressbars_time = 2.f;
			const float progress = Math::Min(float(GetSystemInterface()->GetElapsedTime() - submitting_start_time) / progressbars_time, 2.f);

			float value_gauge = 1.0f;
			float value_horizontal = 0.0f;
			if (progress < 1.0f)
				value_gauge = 0.5f - 0.5f * Math::Cos(Math::RMLUI_PI * progress);
			else
				value_horizontal = 0.5f - 0.5f * Math::Cos(Math::RMLUI_PI * (progress - 1.0f));

			progress_horizontal->SetAttribute("value", value_horizontal);

			const float value_begin = 0.09f;
			const float value_end = 1.f - value_begin;
			float value_mapped = value_begin + value_gauge * (value_end - value_begin);
			gauge->SetAttribute("value", value_mapped);

			auto value_gauge_str = CreateString(10, "%d %%", Math::RoundToInteger(value_gauge * 100.f));
			auto value_horizontal_str = CreateString(10, "%d %%", Math::RoundToInteger(value_horizontal * 100.f));

			if (auto el_value = document->GetElementById("gauge_value"))
				el_value->SetInnerRML(value_gauge_str);
			if (auto el_value = document->GetElementById("progress_value"))
				el_value->SetInnerRML(value_horizontal_str);

			String label = "Placing tubes";
			size_t num_dots = (size_t(progress * 10.f) % 4);
			if (progress > 1.0f)
				label += "... Placed! Assembling message";
			if (progress < 2.0f)
				label += String(num_dots, '.');
			else
				label += "... Done!";

			if (auto el_label = document->GetElementById("progress_label"))
				el_label->SetInnerRML(label);

			if (progress >= 2.0f)
			{
				submitting = false;
				if (auto el_output = document->GetElementById("form_output"))
					el_output->SetInnerRML(submit_message);
			}
		}
	}

	void Shutdown() {
		if (document)
		{
			mode_a.stop();
			document->Close();
			document = nullptr;
		}
	}

	void ProcessEvent(Rml::Event& event) override
	{
		using namespace Rml;

		switch (event.GetId())
		{
		case EventId::Keydown:
		{
			Rml::Input::KeyIdentifier key_identifier = (Rml::Input::KeyIdentifier)event.GetParameter< int >("key_identifier", 0);

			if (key_identifier == Rml::Input::KI_ESCAPE)
				Backend::RequestExit();
		}
		break;

		default:
			break;
		}
	}

	Rml::ElementDocument* GetDocument() {
		return document;
	}

	void SetSandboxStylesheet(const Rml::String& string)
	{
		if (iframe && rml_basic_style_sheet)
		{
			auto style = Rml::MakeShared<Rml::StyleSheetContainer>();
			Rml::StreamMemory stream((const Rml::byte*)string.data(), string.size());
			stream.SetSourceURL("sandbox://rcss");

			style->LoadStyleSheetContainer(&stream);
			style = rml_basic_style_sheet->CombineStyleSheetContainer(*style);
			iframe->SetStyleSheetContainer(style);
		}
	}

	void SetSandboxBody(const Rml::String& string)
	{
		if (iframe)
		{
			iframe->SetInnerRML(string);
		}
	}

	void updateWorkModel(bool flag) {
		if (flag)
		{
			if (flag != is_auto) {
				is_auto = flag;
				mode_a.start();
			}
		}
		else {
			if (flag != is_auto) {
				is_auto = flag;
				mode_a.stop();
			}
		}
	}

private:
	Rml::ElementDocument* document = nullptr;
	Rml::ElementDocument* iframe = nullptr;
	Rml::Element* gauge = nullptr, * progress_horizontal = nullptr;
	Rml::SharedPtr<Rml::StyleSheetContainer> rml_basic_style_sheet;

	bool submitting = false;
	double submitting_start_time = 0;
	Rml::String submit_message;

	// true:自动模式，false:手动模式
	bool is_auto = true;
	// 自动模式下缓冲对象
	model_automation mode_a;

};


Rml::UniquePtr<DemoWindow> demo_window;

struct TweeningParameters {
	Rml::Tween::Type type = Rml::Tween::Linear;
	Rml::Tween::Direction direction = Rml::Tween::Out;
	float duration = 0.5f;
} tweening_parameters;

class DemoEventListener : public Rml::EventListener
{
public:
	DemoEventListener(const Rml::String& value, Rml::Element* element) : value(value), element(element) {}

	void ProcessEvent(Rml::Event& event) override
	{
		using namespace Rml;

		if (value == "auto_model") {
			demo_window->updateWorkModel(true);
		}
		else if (value == "config_model") {
			demo_window->updateWorkModel(false);
		}
		else if (value == "rating")
		{
			auto el_rating = element->GetElementById("rating");
			auto el_rating_emoji = element->GetElementById("rating_emoji");
			if (el_rating && el_rating_emoji)
			{
				enum { Sad, Mediocre, Exciting, Celebrate, Champion, CountEmojis };
				static const Rml::String emojis[CountEmojis] = {
					(const char*)u8"😢", (const char*)u8"😐", (const char*)u8"😮",
					(const char*)u8"😎", (const char*)u8"🏆"
				};
				int value = event.GetParameter("value", 50);

				Rml::String emoji;
				if (value <= 0)
					emoji = emojis[Sad];
				else if (value < 50)
					emoji = emojis[Mediocre];
				else if (value < 75)
					emoji = emojis[Exciting];
				else if (value < 100)
					emoji = emojis[Celebrate];
				else
					emoji = emojis[Champion];

				el_rating->SetInnerRML(Rml::CreateString(30, "%d%%", value));
				el_rating_emoji->SetInnerRML(emoji);
			}
		}
		else if (value == "submit_form")
		{
			const auto& p = event.GetParameters();
			Rml::String output = "<p>";
			for (auto& entry : p)
			{
				auto value = Rml::StringUtilities::EncodeRml(entry.second.Get<Rml::String>());
				if (entry.first == "message")
					value = "<br/>" + value;
				output += "<strong>" + entry.first + "</strong>: " + value + "<br/>";
			}
			output += "</p>";

		}
		else if (value == "set_sandbox_body")
		{
			if (auto source = static_cast<Rml::ElementFormControl*>(element->GetElementById("sandbox_rml_source")))
			{
				auto value = source->GetValue();
				demo_window->SetSandboxBody(value);
			}
		}
		else if (value == "set_sandbox_style")
		{
			if (auto source = static_cast<Rml::ElementFormControl*>(element->GetElementById("sandbox_rcss_source")))
			{
				auto value = source->GetValue();
				demo_window->SetSandboxStylesheet(value);
			}
		}
	}

	void OnDetach(Rml::Element* /*element*/) override { delete this; }

private:
	Rml::String value;
	Rml::Element* element;
};



class DemoEventListenerInstancer : public Rml::EventListenerInstancer
{
public:
	Rml::EventListener* InstanceEventListener(const Rml::String& value, Rml::Element* element) override
	{
		return new DemoEventListener(value, element);
	}
};


#if defined RMLUI_PLATFORM_WIN32
#include "backends/RmlUi_Include_Windows.h"
int APIENTRY WinMain(HINSTANCE /*instance_handle*/, HINSTANCE /*previous_instance_handle*/, char* /*command_line*/, int /*command_show*/)
#else
int main(int /*argc*/, char** /*argv*/)
#endif
{
	const int width = 1024;
	const int height = 768;

	// 全局日志声明
	Log::SPDLOG::getInstance().init("logger", "DEBUG", "DEBUG", 10 * 1024 * 1024 * 100, 10, false);
	// Initializes the shell which provides common functionality used by the included samples.
	Log::SPDLOG::LOG_DEBUG("Shell::Initialize");
	if (!Shell::Initialize())
		return -1;

	if (!parse_sensor()) {
		return -1;
	}

	// Constructs the system and render interfaces, creates a window, and attaches the renderer.
	if (!Backend::Initialize("P2000 Demo", width, height, false))
	{
		Shell::Shutdown();
		return -1;
	}

	// Install the custom interfaces constructed by the backend before initializing RmlUi.
	Rml::SetSystemInterface(Backend::GetSystemInterface());
	Rml::SetRenderInterface(Backend::GetRenderInterface());

	// RmlUi initialisation.
	Rml::Initialise();

	// Create the main RmlUi context.
	Rml::Context* context = Rml::CreateContext("main", Rml::Vector2i(width, height));
	if (!context)
	{
		Rml::Shutdown();
		Backend::Shutdown();
		Shell::Shutdown();
		return -1;
	}
	
	Rml::Debugger::Initialise(context);

	DemoEventListenerInstancer event_listener_instancer;
	Rml::Factory::RegisterEventListenerInstancer(&event_listener_instancer);

	Shell::LoadFonts();

	demo_window = Rml::MakeUnique<DemoWindow>("Demo sample1", context);
	demo_window->GetDocument()->AddEventListener(Rml::EventId::Keydown, demo_window.get());
	demo_window->GetDocument()->AddEventListener(Rml::EventId::Keyup, demo_window.get());
	demo_window->GetDocument()->AddEventListener(Rml::EventId::Animationend, demo_window.get());

	bool running = true;
	while (running)
	{
		running = Backend::ProcessEvents(context, &Shell::ProcessKeyDownShortcuts, true);

		demo_window->Update();
		context->Update();

		Backend::BeginFrame();
		context->Render();
		Backend::PresentFrame();
	}

	demo_window->Shutdown();

	// Shutdown RmlUi.
	Rml::Shutdown();

	Backend::Shutdown();
	Shell::Shutdown();

	demo_window.reset();

	return 0;
}






