class ObjContent
{
public:
	ObjContent() {}
	std::vector<ModelTriangle> faces;
	std::unordered_map<std::string, Colour> palette;
	void addFace(ModelTriangle face)
	{
		faces.push_back(face);
	}

	void addColour(Colour col, std::string name)
	{
		palette[name] = col;
	}
};


void populatePalette(ObjContent content, string filename)
{
	ofstream myfile;
	string line;
	Colour currentCol = Colour(0, 0, 0);
	std::ifstream in(filename, std::ios::in);
	if (!in)
	{
		std::cerr << "Cannot open " << filename << std::endl;
		exit(1);
	}
	string key;
	while (std::getline(in, line))
	{
		if (line.size() > 4)
		{
			if (line.substr(0, 6) == "newmtl")
			{
				key = line.substr(7, line.size());
				currentCol = content.palette[key];
			}
			else if (line.substr(0, 2) == "Kd")
			{
				string *stuff = split(line, ' ');
				int r = 255 * stod(stuff[1]);
				int g = 255 * stod(stuff[2]);
				int b = 255 * stod(stuff[3]);
				// std::cout << key << std::endl;
				// printf("\n %i,%i,%i\n", r, g, b);
				content.addColour(Colour(r, g, b), key);
			}
		}
	}
}
ObjContent objFileRead(string filename)
{
	ObjContent toReturn = ObjContent();
	vector<vec3> vertices;
	populatePalette(toReturn, "cornell-box.mtl");
	string line;
	Colour currentCol = Colour(0, 0, 0);
	std::ifstream in(filename, std::ios::in);
	if (!in)
	{
		std::cerr << "Cannot open " << filename << std::endl;
		exit(1);
	}
	while (std::getline(in, line))
	{
		//check v for vertices
		if (line.size() > 4)
		{
			if (line.substr(0, 6) == "usemtl")
			{
				string key = line.substr(7, line.size());
				currentCol = toReturn.palette[key];
			}
			else if (line.substr(0, 2) == "v ")
			{
				string *stuff = split(line, ' ');
				int x = stod(stuff[1]);
				int y = stod(stuff[2]);
				int z = stod(stuff[3]);
				vertices.push_back(vec3(x, y, z));
			}
			//check for faces
			else if (line.substr(0, 2) == "f ")
			{
				int A, B, C; //to store vertex indices
				const char *chh = line.c_str();
				sscanf(chh, "f %i/ %i/ %i/", &A, &B, &C); //here it read the line start with f and store the corresponding values in the variables
				A--;
				B--;
				C--;
				ModelTriangle t = ModelTriangle(vertices.at(A), vertices.at(B), vertices.at(C), currentCol);
				toReturn.addFace(t);
			}
		}
	}
	return toReturn;
}