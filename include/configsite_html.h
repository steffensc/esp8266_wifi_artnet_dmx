// - - - - - String Placholder Filling - - - - - - - -
// This uses lots of extra Memory, approx. 14%, only for doing "the string placeholders thing"
// ...But that's the price you pay for this kind/style of code using c++11 :D
#include <memory>
#include <stdexcept>

template<typename ... Args>
std::string string_format( const std::string& format, Args ... args )
{
    int size_s = std::snprintf( nullptr, 0, format.c_str(), args ... ) + 1; // Extra space for '\0'
    if( size_s <= 0 ){ throw std::runtime_error( "Error during formatting." ); }
    auto size = static_cast<size_t>( size_s );
    auto buf = std::make_unique<char[]>( size );
    std::snprintf( buf.get(), size, format.c_str(), args ... );
    return std::string( buf.get(), buf.get() + size - 1 ); // We don't want the '\0' inside
}
// - - - - - - - - - - - - - - - - - - - - -- - - - - - 


std::string configsite_config_html =
"<!DOCTYPE HTML>"
"<html>"
"<head>"
"<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
"<style type='text/css'>"
"body{font-family: sans-serif;}"
"form input{font-size: 12pt; display: block; width:  100%; border: 1px solid black; padding: 5px 0px; margin-bottom: 10px;}"
"</style>"
"</head>"
"<body>"
"<h1>%s<br>%s</h1>"
"<br>"
"<p>"
"<h3>Current top 5 WiFi-Networks in reach:</h3>"
"<ol>%s</ol>"
"</p>"
"<br>"
"<h3>Configure new WiFi:</h3>"
"<form method='get' action='setting' style='width:100%;'>"
"<label>SSID:</label>"
"<input name='ssid' length=32>"
"<br>"
"<label>PASS:</label>"
"<input name='pass' length=64>"
"<br>"
"<input type='submit'>"
"</form>"
"</body>"
"</html>";

std::string configsite_success_html =
"<!DOCTYPE HTML>"
"<html>"
"<head>"
"<style type='text/css'>"
"body{font-family: sans-serif;}"
"</style>"
"</head>"
"<body>"
"<h1>Success!</h1>"
"<br>"
"Data saved to EEPROM."
"<br><br>"
"...resetting for boot into new WiFi."
"</body>"
"</html>";

std::string configsite_error_html =
"<!DOCTYPE HTML>"
"<html>"
"<head>"
"<style type='text/css'>"
"body{font-family: sans-serif;}"
"</style>"
"</head>"
"<body>"
"<h1>ERROR!</h1>"
"<br>"
"Data could not be saved to EEPROM."
"</body>"
"</html>";