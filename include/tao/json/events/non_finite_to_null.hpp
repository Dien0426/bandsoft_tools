// Copyright (c) 2017 Dr. Colin Hirsch and Daniel Frey
// Please see LICENSE for license or visit https://github.com/taocpp/json/

#ifndef TAOCPP_JSON_INCLUDE_EVENTS_NON_FINITE_TO_NULL_HPP
#define TAOCPP_JSON_INCLUDE_EVENTS_NON_FINITE_TO_NULL_HPP

#include <cmath>

namespace tao
{
   namespace json
   {
      namespace events
      {
         template< typename Consumer >
         struct non_finite_to_null
            : public Consumer
         {
            using Consumer::Consumer;

            using Consumer::number;

            void number( const double v )
            {
               if( !std::isfinite( v ) ) {
                  Consumer::null();
               }
               else {
                  Consumer::number( v );
               }
            }
         };

      }  // namespace events

   }  // namespace json

}  // namespace tao

#endif
