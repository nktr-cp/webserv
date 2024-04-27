#ifndef LOCATION_HPP_
#define LOCATION_HPP_

#include "Typedefs.hpp"

class Location {
	private:
		//parsed
		std::string uri_;
		short methods_;
		std::string root_;
		bool autoindex_;
		std::string index_;
		std::vector<std::string> extensions_;
		std::string upload_path_;
		std::string redirect_;

	public:
		// setter, getter
		Location();
		void setUri(const std::string&);
		void addMethod(const HttpMethod);
		void setRoot(const std::string&);
		void setAutoindex(bool);
		void setIndex(const std::string&);
		void addExtension(const std::string&);
		void setUploadPath(const std::string&);
		void setRedirect(const std::string&);

		const std::string&				getUri() const;
		short						getMethods() const;
		const std::string&				getRoot() const;
		bool						getAutoindex() const;
		const std::string&				getIndex() const;
		const std::vector<std::string>&	getExtensions() const;
		const std::string&				getUploadPath() const;
		const std::string&				getRedirect() const;
		//debug
		void print();
};

#endif // LOCATION_HPP_
