/*!
 * wtengine | File:  exceptions.hpp
 * 
 * \author Matthew Evans
 * \version 0.7.2
 * \copyright See LICENSE.md for copyright information.
 * \date 2019-2022
 */

#ifndef WTE_EXCEPTION_HPP
#define WTE_EXCEPTION_HPP

#include <string>
#include <fstream>
#include <exception>

#include "wtengine/_globals/_defines.hpp"
#include "wtengine/_globals/engine_time.hpp"

namespace wte {

class logger;

/*!
 * \class
 */
class exception_item final {
    friend class runtime_error;
    friend class exception;

    public:
        /*!
         * \brief
         */
        exception_item(const std::string& d, const std::string& l, const uint& c);

    private:
        const char* description;  //  Exception description.
        const char* location;     //  Exception location.
        const uint code;          //  Code of error
        const int64_t time;       //  Time of exception.
};

/*!
 * \class runtime_error
 * \brief Throws an engine runtime error.
 *
 * Exceptions thrown this way will terminate the engine.
 */
class runtime_error final : public std::exception {
    public:
        /*!
         * \brief Create a new runtime error object.
         * \param i An exception item object.
         */
        runtime_error(const exception_item& i);

        runtime_error() = delete;    //!<  Delete default constructor.

        /*!
         * \brief Terminate engine after handling exception.
         */
        virtual ~runtime_error() {
            std::exit(item.code);
        };

        /*!
         * \brief Returns the description of the thrown exception.
         * \return Description of thrown exception.
         */
        const char* what() const noexcept override;

        /*!
         * \brief Return the location the exception occured.
         * \return Location of thrown exception.
         */
        const char* where() const noexcept;

        /*!
         * \brief Return the time the exception occured.
         * \return Time of thrown exception.
         */
        const int64_t when() const noexcept;

    private:
        const exception_item item;  //  Store the exception item.
};

/*!
 * \class exception
 * \brief Throws an internal engine exception.
 *
 * Exceptions thrown this way will not terminate the engine. \n
 * If debugging is enabled, they will also be logged to file.
 */
class exception final : public std::exception {
    public:
        /*!
         * \brief Create a wte exception.
         * \param i An exception item object.
         */
        exception(const exception_item& i);

        exception() = delete;    //!<  Delete default constructor.
        ~exception() = default;  //!<  Default destructor.

        /*!
         * \brief Returns the description of the thrown exception.
         * \return Description of thrown exception.
         */
        const char* what() const noexcept override;

        /*!
         * \brief Return the location the exception occured.
         * \return Location of thrown exception.
         */
        const char* where() const noexcept;

        /*!
         * \brief Return the time the exception occured.
         * \return Time of thrown exception.
         */
        const int64_t when() const noexcept;

    private:
        const exception_item item;  //  Store the exception item.
};

}  //  end namespace wte

#endif
