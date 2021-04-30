# Assignment 4
I kinda made a RESTful API I guess. By default runs on 0.0.0.0:8888. Works over HTTP, the server is running in several threads. Database used: sqlite3 with fnc12/sqlite_orm.

## Status codes
For successfull requests the status code is 200. Status code 400 means that most likely something was wrong with the request, e.g. some of the fields are missing. In case of status code 400 server returns a JSON object with a single field `error_message` (string) for a better understanding of what exactly went wrong.

## API endpoints
[GET] or [POST] indicates which HTTP method is to be used for the endpoint listed.
### Passenger API
#### [POST] `/passenger/signup` 
Sign up as a passenger.
Takes a JSON object in the HTTP body of the request that must have fields `first_name` (string), `last_name` (string), `login` (string) and `password` (string).
#### [POST] `/passenger/login` 
Log in as a passenger. 
Takes a JSON object with two fields: `login` (string) and `password` (string).
Returns a token as a JSON object with only one field which is `token` (string). Token is to be used like so when making requests to the rest of endpoints: add `Authorization: Bearer *your_token*` header to your HTTP request.
#### [GET] `/passenger/order_history` 
Retrieve order history of a passenger. Requires authorization.
Returns order history as a JSON array of objects that have fields `id` (int), `passenger_id` (int), `driver_id` (int), `car_id` (int), `car_type` (string), `route_from` (string), `route_to` (string), `timestamp` (string) and `status` (string).
#### [GET] `/passenger/payment_methods` 
List all the available payment methods of a passenger. Requres authorization.
Returns payment methods as a JSON array of objects that have fields `id` (int), `card_number` (string) (first 12 digits of the card number are omitted), `year` (int), `month` (int), `first_name` (string), `last_name` (string).
#### [POST] `/passenger/payment_methods` 
Add a payment method for a passenger. Requires authorization.
Takes a JSON object that must have fields `card_number` (string), `cvc` (int), `month` (int), `year` (int), `first_name` (string), `last_name` (string).
#### [GET] `/passenger/pinned_addresses`
List pinned addresses of a passenger. Requires authorization.
Returns pinned address as a JSON array of strings.
#### [POST] `/passenger/pinned_addresses`
Add new pinned addresses. Requires authorization.
Takes a JSON array of strings.
#### [POST] `/passenger/calculate_price`
Gives an estimated price for a specified ride. Requires authorization.
Takes a JSON object with fields `from` (string), `to` (string) and `car_type` (string).
#### [POST] `/passenger/place_order`
Place an order for a specified ride. Requires authorization.
Takes a JSON object with fields `from` (string), `to` (string), `car_type` (string) and `payment_method` (int). `payment_method` field should be an id of one of the payment methods current user has. 
### Driver API
#### [POST] `/driver/signup` 
Sign up as a driver.
Takes a JSON object with two fields: `car` and `driver`. `car` is expected to be a JSON object with fields `model` (string), `registration_plate` (string), `type` (string). `driver` is expected to be a JSON object with fields `login` (string), `password` (string), `first_name` (string), `last_name` (string). 
#### [POST] `/driver/login` 
Log in as a driver. 
Takes a JSON object with two fields: `login` (string) and `password` (string).
Returns a token as a JSON object with only one field which is `token` (string). Token is to be used like so when making requests to the rest of endpoints: add `Authorization: Bearer *your_token*` header to your HTTP request.
#### [GET] `/driver/car`
View information about driver's car. Requires authorization.
Returns a JSON object with fields `model` (string), `registration_plate` (string) and `type` (string).
#### [GET] `/driver/order_history`
Retrieve order history of a passenger. Requires authorization.
Returns order history as a JSON array of objects that have fields `id` (int), `passenger_id` (int), `driver_id` (int), `car_id` (int), `car_type` (string), `route_from` (string), `route_to` (string), `timestamp` (string) and `status` (string).
#### [POST] `/driver/update_status`
Update driver's status. Requires authorization.
Takes a JSON object with a single field `is_working` (bool).
#### [GET] `/driver/available_orders`
Lists all available orders at the moment, taking driver's car type into consideration. Requires authorization.
Returns a JSON array with objects that have fields `id` (int), `from` (string), `to` (string), `price` (int).
#### [POST] `/driver/pick_order`
Pick order by its id. Requires authorization.
Takes a JSON object with a single field `order_id` (int).
#### [POST] `/driver/finish_order`
Finish the ongoing ride. Requires authorization.
Does not take or return anything.
