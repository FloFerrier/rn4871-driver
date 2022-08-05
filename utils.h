/**
 * \file    utils.h
 * \defgroup Utils Tools used by RN4871 driver and the virtual module
 * \copyright nothing
 *
 * \brief   No description.
 * \details Nothing yet.
 *
 * @{
 */

#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>
#include <string.h>

/*!
 * \details This API allows to return if a string is a hexadecimal value encoded on ASCII or not.
 *
 * \param[in] hexa : String that contains hexadecimal character
 * \param[in] size : Size of hexa string
 *
 * \return Return a boolean
 * \retval false
 * \retval true
 */
bool checkHexaIsCorrect(const char *hexa, size_t size);

#endif /* UTILS_H */

/**@}*/