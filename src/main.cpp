#include <rapidxml/rapidxml.hpp>
#include <rapidxml/rapidxml_utils.hpp>
#include <rapidxml/rapidxml_print.hpp>
#include <stdlib.h>
#include <math.h>
#include <time.h>

double get_lat(rapidxml::xml_node<>* trkpt)
{
	rapidxml::xml_attribute<>* attr = trkpt->first_attribute("lat");
	if (!attr)
	{
		return 0.0f;
	}

	return atof(attr->value());
}

double get_lon(rapidxml::xml_node<>* trkpt)
{
	rapidxml::xml_attribute<>* attr = trkpt->first_attribute("lon");
	if (!attr)
	{
		return 0.0f;
	}

	return atof(attr->value());
}

double deg2rad(double deg)
{
	return deg * M_PI / 180.0;
}

double calc_dist(rapidxml::xml_node<>* trkpt, rapidxml::xml_node<>* trkpt_prev)
{
	double lat = deg2rad(90.0 - get_lat(trkpt));
	double lon = deg2rad(get_lon(trkpt));
	double lat_prev = deg2rad(90.0 - get_lat(trkpt_prev));
	double lon_prev = deg2rad(get_lon(trkpt_prev));
	double c = sin(lat) * sin(lat_prev) * cos(lon - lon_prev) + cos(lat) * cos(lat_prev);
	return 6378.137 * acos(c);
}

int main(int argc, char* argv[])
{
	if (argc < 3 || argc > 4)
	{
		puts("Usage: gpx2garmin <input_file> <output_file> [speed]");
		return -1;
	}

	rapidxml::file<> fin(argv[1]);
	rapidxml::xml_document<> in;
	in.parse<0>(fin.data());
	rapidxml::xml_node<>* gpx_in = in.first_node("gpx");

	if (!gpx_in)
	{
		printf("\"%s\" is not a gpx file.", argv[1]);
		return -1;
	}

	time_t stamp = time(0);
	double speed = 35.0 / 3600.0;
	if (argc == 4)
	{
		speed = atof(argv[3]) / 3600.0;
	}

	rapidxml::xml_document<> out;
	rapidxml::xml_node<>* decl_out = in.allocate_node(rapidxml::node_declaration);
	decl_out->append_attribute(in.allocate_attribute("version", "1.0"));
	decl_out->append_attribute(in.allocate_attribute("encoding", "UTF-8"));
	out.append_node(decl_out);

	rapidxml::xml_node<>* gpx_out = in.allocate_node(rapidxml::node_element, "gpx");
	gpx_out->append_attribute(in.allocate_attribute("version", "1.1"));
	gpx_out->append_attribute(in.allocate_attribute("creator", "Garmin Connect"));
	gpx_out->append_attribute(in.allocate_attribute("xsi:schemaLocation", "http://www.topografix.com/GPX/1/1 http://www.topografix.com/GPX/1/1/gpx.xsd http://www.garmin.com/xmlschemas/GpxExtensions/v3 http://www.garmin.com/xmlschemas/GpxExtensionsv3.xsd http://www.garmin.com/xmlschemas/TrackPointExtension/v1 http://www.garmin.com/xmlschemas/TrackPointExtensionv1.xsd"));
	gpx_out->append_attribute(in.allocate_attribute("xmlns", "http://www.topografix.com/GPX/1/1"));
	gpx_out->append_attribute(in.allocate_attribute("xmlns:gpxtpx", "http://www.garmin.com/xmlschemas/TrackPointExtension/v1"));
	gpx_out->append_attribute(in.allocate_attribute("xmlns:gpxx", "http://www.garmin.com/xmlschemas/GpxExtensions/v3"));
	gpx_out->append_attribute(in.allocate_attribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance"));

	rapidxml::xml_node<>* metadata = in.allocate_node(rapidxml::node_element, "metadata");
	rapidxml::xml_node<>* metadata_link = in.allocate_node(rapidxml::node_element, "link");
	metadata_link->append_attribute(in.allocate_attribute("href", "connect.garmin.com"));
	metadata_link->append_node(in.allocate_node(rapidxml::node_element, "text", "Garmin Connect"));
	metadata->append_node(metadata_link);
	char* time_value = in.allocate_string(0, 21);
	strftime(time_value, 21, "%Y-%m-%dT%H:%M:%SZ", gmtime(&stamp));
	metadata->append_node(in.allocate_node(rapidxml::node_element, "time", time_value));
	gpx_out->append_node(metadata);

	for (rapidxml::xml_node<>* trk_in = gpx_in->first_node("trk"); trk_in; trk_in = trk_in->next_sibling("trk"))
	{
		rapidxml::xml_node<>* trk_out = in.allocate_node(rapidxml::node_element, "trk");
		trk_out->append_node(in.allocate_node(rapidxml::node_element, "name", argv[2]));
		for (rapidxml::xml_node<>* trkseg_in = trk_in->first_node("trkseg"); trkseg_in; trkseg_in = trkseg_in->next_sibling("trkseg"))
		{
			rapidxml::xml_node<>* trkseg_out = in.clone_node(trkseg_in);
			rapidxml::xml_node<>* trkpt_prev = 0;
			int stamp_inc = 0;
			for (rapidxml::xml_node<>* trkpt_out = trkseg_out->first_node("trkpt"); trkpt_out; trkpt_out = trkpt_out->next_sibling("trkpt"))
			{
				if (trkpt_prev)
				{
					double dist = calc_dist(trkpt_out, trkpt_prev);
					double tv = dist / speed;
					stamp_inc += tv < 1.0 ? 1 : tv;
				}

				trkpt_prev = trkpt_out;
				time_t stamp_out = stamp + stamp_inc;
				time_value = in.allocate_string(0, 21);
				strftime(time_value, 21, "%Y-%m-%dT%H:%M:%SZ", gmtime(&stamp_out));
				trkpt_out->append_node(in.allocate_node(rapidxml::node_element, "time", time_value));
			}
			trk_out->append_node(trkseg_out);
		}
		gpx_out->append_node(trk_out);
	}

	out.append_node(gpx_out);

	std::ofstream fout(argv[2]);
	fout << out;

	puts("Complete!");

	return 0;
}
