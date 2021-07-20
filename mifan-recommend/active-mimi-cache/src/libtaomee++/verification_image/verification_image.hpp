/**
 *============================================================
 *  @file      verification_image.hpp
 *  @brief    generate verification image. libgd2-xpm-dev needed: -lgd
 * 
 *  compiler   gcc4.1.2
 *  platform   Linux
 *
 *  copyright:  TaoMee, Inc. ShangHai CN. All rights reserved.
 *
 *============================================================
 */

#ifndef LIBTAOMEEPP_VERIFICATION_IMAGE_HPP_
#define LIBTAOMEEPP_VERIFICATION_IMAGE_HPP_

#include <stdexcept>
#include <string>
#include <vector>

extern "C" {
#include <gd.h>
}


namespace taomee {

/**
 * @brief Exception thrown by VerificationImageGenerator
 */
class VerifImgError : public std::logic_error {
public:
	explicit VerifImgError(const std::string& errmsg) : std::logic_error(errmsg)
		{ }
};

/**
 * @brief Holds a verification code and the related verification image returned from VerificationImageGenerator::generate
 */
class VerificationImage {
private:
	friend class VerificationImageGenerator;

private:
	explicit VerificationImage(const char* code, const char* img, int size)
				: m_verif_code(code), m_img(img, size)
		{ }

public:
	/**
	  * @brief return a string holding the verification code
	  */
	const std::string& get_verif_code() const
		{ return m_verif_code; }
	/**
	  * @brief return a verification image related to the verification code
	  */
	const std::string& get_image() const
		{ return m_img; }

private:
	std::string	m_verif_code;
	std::string	m_img;
};

/**
  * @brief for generating a verification image
  */
class VerificationImageGenerator {
public:
	/**
	  * @brief ctor
	  * @param font_dir A directory that holds some ttf/ttc files. 'generate()' will use these ttf/ttc files randomly for
	  *                        generating a verification image
	  * @param length length of the generated verification image
	  * @param width width of the generated verification image
	  * @param fontsize font size in the generated image
	  * @throw VerifImgError
	  */
	VerificationImageGenerator(const char* font_dir, int length = 79, int width = 28, int fontsize = 20);

	/**
	* @brief generate a verification image
	* @param nchars number of characters written to a verification image
	* @param is_easy whether the generated image is easy to recognize or not
	* @return a verification image
	* @throw VerifImgError
	*/
	VerificationImage generate(int nchars, bool is_easy);

private:
	// check if the given file is a valid true type font file
	bool is_valid_ttf(const char* filename);
	// generate some simple noisy elements
	void simple_noisy(gdImagePtr im, int color);
	// generate some complicated noisy elements
	void complicated_noisy(gdImagePtr im);

private:
	static const int	sc_verif_code_num = 9;

private:
     const int cm_length;
	 const int cm_width;
     const int cm_fontsize;

private:
	std::vector<std::string>	m_fontpath;

private:
	static const char sm_simple_meta_chars[];
	// omit some char which you may misread it for other chars (such as: O 1 l V v)
	static const char sm_meta_chars[];
	// some thin chars to save space
	static const char sm_thin_meta_chars[];
	// angle of the chars being written to the verification image
	static const double sm_angles[];
};

}

#endif // LIBTAOMEEPP_VERIFICATION_IMAGE_HPP_

