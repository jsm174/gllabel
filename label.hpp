#include <string>
#include <vector>
#include <memory>
#include <map>
#include <glew.h>
#include <glm/glm.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OUTLINE_H

class GLFontManager
{
public:
	struct AtlasGroup
	{
		// Grid atlas contains an array of square grids with side length
		// gridMaxSize. Each grid takes a single glyph and splits it into
		// cells that inform the fragment shader which curves of the glyph
		// intersect that cell. The cell contains coords to data in the bezier
		// atlas. The bezier atlas contains the actual bezier curves for each
		// glyph. All data for a single glyph must lie in a single row, although
		// multiple glyphs can be in one row. Each bezier curve takes three
		// "RGBA pixels" (12 bytes) of data.
		// Both atlases also encode some extra information, which is explained
		// where it is used in the code.
		GLuint bezierAtlasId, gridAtlasId;
		uint8_t *bezierAtlas, *gridAtlas;
		uint16_t nextBezierPos[2], nextGridPos[2]; // XY pixel coordinates 
		bool full; // For faster checking
		bool uploaded;
	};
	
	struct Glyph
	{
		uint16_t bezierAtlasPos[2]; // XY pixel coordinates
		uint16_t atlasIndex;
		glm::vec2 size;// Width and height in GL units
		float shift; // Amount to shift after character in GL units
	};

public: // TODO: private
	std::vector<AtlasGroup> atlases;
	std::map<FT_Face, std::map<uint32_t, Glyph>> glyphs;
	FT_Library ft;
	GLuint glyphShader, uGridAtlas, uBezierAtlas, uGridTexel, uBezierTexel, uPosScale;
	
	GLFontManager();
	
	AtlasGroup * GetOpenAtlasGroup();
	
public:
	~GLFontManager();

	static std::shared_ptr<GLFontManager> singleton;
	static std::shared_ptr<GLFontManager> GetFontManager();	
	
	FT_Face GetFontFromPath(std::string fontPath);
	FT_Face GetFontFromName(std::string fontName);
	FT_Face GetDefaultFont();
	
	Glyph * GetGlyphForCodepoint(FT_Face face, uint32_t point);
	void LoadASCII(FT_Face face);
	void UploadAtlases();
	
	void UseGlyphShader();
	void SetShaderPosScale(glm::vec4 posScale); // Only to be used after UseGlyphShader called
};

class GLLabel
{
public:
	enum class Align
	{
		Start,
		Center,
		End
	};
	
	struct Color
	{
		uint8_t r,g,b,a;
	};
	
private:
	struct GlyphVertex
	{
		// XY coords of the vertex
		glm::vec2 pos;
		
		// The UV coords of the data for this glyph in the bezier atlas
		// Also contains a vertex-dependent norm coordiate by encoding:
		// encode: data[n] = coord[n] * 2 + norm[n]
		// decode: norm[n] = data[n] % 2,  coord[n] = int(data[n] / 2)
		uint16_t data[2];
		
		// RGBA color [0,255]
		Color color;
	};
	
	std::shared_ptr<GLFontManager> manager;
	std::vector<GlyphVertex> verts;
	GLuint vertBuffer;
	
	std::string text;
	glm::vec2 pos, scale, appendOffset;
	bool showingCaret;
	Align horzAlign;
	Align vertAlign;
	FT_Face lastFace;
	Color lastColor;
		
public:
	GLLabel();
	GLLabel(std::string text);
	~GLLabel();
	
	inline std::string GetText() { return this->text; }

	inline void SetText(std::string text) { SetText(text, lastFace, lastColor); }
	inline void SetText(std::string text, std::string face, Color color) { SetText(text, manager->GetFontFromName(face), color); }
	void SetText(std::string text, FT_Face face, Color color);
	
	inline void AppendText(std::string text) { AppendText(text, lastFace, lastColor); }
	inline void AppendText(std::string text, std::string face, Color color) { SetText(text, manager->GetFontFromName(face), color); }
	void AppendText(std::string text, FT_Face face, Color color);

	void SetPosition(float x, float y) { this->pos = glm::vec2(x,y); }
	void SetScale(float x, float y) { this->scale = glm::vec2(x,y); }
	void SetHorzAlignment(Align horzAlign);
	void SetVertAlignment(Align vertAlign);
	void ShowCaret(bool show) { showingCaret = show; }
	
	// Render the label. Also uploads modified textures as necessary. 'time'
	// should be passed in monotonic seconds (no specific zero time necessary).
	void Render(float time);
	
	// In the case that multiple GLLabels are rendered in immediate succession,
	// the first label should be rendered by calling GLLabel::Render and
	// then the rest can be rendered faster by calling GLLabel::RenderAlso.
	void RenderAlso(float time);
};