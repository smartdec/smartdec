/* This file was copied from Combild with permission of the copyright holder. 
 * See http://combild.net. 
 * 
 * Minimal changes were made to make the code conform to nc naming conventions. */

/* Combild
 * 
 * Copyright (c) 2011 Alexander Fokin */
#ifndef CB_CORE_WARNINGS_H
#define CB_CORE_WARNINGS_H

#define NC_FUNCTION_MAX_ARITY 10

#include <nc/config.h>
#include <boost/preprocessor/repetition/enum_binary_params.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/repetition/repeat_from_to.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <QString>
//#include <arx/utility/Unused.h>
#include "Unused.h"

namespace nc { namespace detail {

    inline void warningInternal(const char *functionName, const QString &s) {
        qWarning("%s: %s", functionName, qPrintable(s));
    }

    inline void criticalInternal(const char *functionName, const QString &s) {
        qCritical("%s: %s", functionName, qPrintable(s));
    }

    inline void fatalInternal(const char *functionName, const QString &s) {
        qFatal("%s: %s", functionName, qPrintable(s));
    }

    typedef void (*WarningHandler)(const char *, const QString &);

    namespace operators {

        template<class T>
        QString operator<<(const QString &s, const T &arg) {
            return s.arg(arg);
        }

        inline QString operator<<(const QString &s, const char *arg) {
            return s.arg(QLatin1String(arg));
        }

#define NC_DEFINE_INVOKE_FUNCTION(Z, N, D)                                      \
        template<BOOST_PP_ENUM_PARAMS(N, class T)>                              \
        void invokeInternal(const WarningHandler &handler, const char *functionName, const QString &s, BOOST_PP_ENUM_BINARY_PARAMS(N, const T, &arg)) {  \
            QString result = s;                                                 \
            BOOST_PP_ENUM_PARAMS(N, (void) 0; result = result << arg);          \
            handler(functionName, result);                                      \
        }
  
        BOOST_PP_REPEAT_FROM_TO(1, NC_FUNCTION_MAX_ARITY, NC_DEFINE_INVOKE_FUNCTION, ~)

        inline void invokeInternal(const WarningHandler &handler, const char *functionName, const QString &s) {
            handler(functionName, s);
        }

    } // namespace operators


    template<class T>
    const char *nullName(const T *) {
        return "NULL";
    }

    template<class T>
    const char *nullName(const T &) {
        return "null";
    }

}} // namespace nc::detail



#define ncWarning(MSG, ...)                                                      \
    ::nc::detail::operators::invokeInternal(&::nc::detail::warningInternal,  Q_FUNC_INFO, QString(MSG), ##__VA_ARGS__)

#define ncCritical(MSG, ...)                                                     \
    ::nc::detail::operators::invokeInternal(&::nc::detail::criticalInternal, Q_FUNC_INFO, QString(MSG), ##__VA_ARGS__)

#define ncFatal(MSG, ...)                                                        \
    ::nc::detail::operators::invokeInternal(&::nc::detail::fatalInternal,    Q_FUNC_INFO, QString(MSG), ##__VA_ARGS__)



#define NC_NULL_PARAMETER_I(MACRO, PARAMETER) {                                 \
    NC_UNUSED(PARAMETER); /* Show compilation error if parameter name is mistyped. */ \
    MACRO("Unexpected %1 parameter '%2'.", ::nc::detail::nullName(PARAMETER), BOOST_PP_STRINGIZE(PARAMETER)); \
}

/**
 * Emits an "unexpected NULL parameter" warning.
 * 
 * \param PARAMETER                    Parameter that was found out to be NULL.
 */
#define ncNullWarning(PARAMETER) NC_NULL_PARAMETER_I(ncWarning, PARAMETER)

/**
 * Emits an "unexpected NULL parameter" critical message.
 * 
 * \param PARAMETER                    Parameter that was found out to be NULL.
 */
#define ncNullCritical(PARAMETER) NC_NULL_PARAMETER_I(ncCritical, PARAMETER)

#endif // CB_CORE_WARNINGS_H
