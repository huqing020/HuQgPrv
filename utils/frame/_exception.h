#ifndef	__EXCEPTION_H__
#define	__EXCEPTION_H__

#include <exception>
#include <string>

class Exception
	: public std::exception
{
///	构造和析构
public:
	explicit Exception( const char* what );
	explicit Exception( const std::string& what );
	virtual ~Exception() throw();

public:
	virtual	const char* what() const throw();
	const char* stackTrace() const throw();

private:
	void	fillStackTrace();

private:
	std::string	_message;
	std::string _stack;
};

#endif	/*__EXCEPTION_H__*/

