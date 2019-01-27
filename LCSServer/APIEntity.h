#ifndef APIENTITY_H
#define APIENTITY_H

#include <QObject>
#include <QTcpSocket>

#include "APIRequest.h"
#include "APIResponse.h"
/**
 * @api {get} /api/entity/[EntityName]:where Fetch
 * @apiName FetchEntity
 * @apiGroup Entity
 *
 * @apiParam {String} [fieldName]  Any combination of FieldName=value pairs can be passed to filter the list returned.  Multiple field/value pairs are joined as an "AND" in the generated WHERE clause.
 * @apiDescription Fetches records from the [EntityName] table with the optional filters applied.
 * @apiExample This example fetches all controllerModules with an address of 2:
 * http://localhost:8080/api/entity/controllerModule?address=2
 * @apiSuccessExample {json} Success-Response:
 *      HTTP/1.1 200 OK
 *      [{
 *              "address": "2",
 *              "controllerID": "24",
 *              "disable": "0",
 *              "id": "6",
 *              "moduleClass": "1",
 *              "moduleName": "TY30-4"
 *          }, {
 *              "address": "2",
 *              "controllerID": "31",
 *              "disable": "0",
 *              "id": "18",
 *              "moduleClass": "9",
 *              "moduleName": "CA Panel1 Output 2"
 *          }, {
 *              "address": "2",
 *              "controllerID": "34",
 *              "disable": "0",
 *              "id": "23",
 *              "moduleClass": "9",
 *              "moduleName": "CA Aux Panel Output 2"
 *          }
 *      ]
 */

/**
 * @api {put} /api/entity/[EntityName] Update an Entity
 * @apiName UpdateEntity
 * @apiGroup Entity
 *
 * @apiDescription Updates a specific record in the [EntityName] table.  The entity is sent as a JSON document.
 * @apiExample This example updates a "route" with an ID of 7.  Note:  You only need to supply the fields that are modified, however like this example, you may include all fields. :
 * http://localhost:8080/api/entity/route
 *
 *     body:
 *     {
 *       "id": 7,
 *       "routeName": "CA 1",
 *       "routeDescription": "New Description"
 *     }
 */

/**
 * @api {post} /api/entity/[EntityName] Add a new Entity
 * @apiName AddEntity
 * @apiGroup Entity
 *
 * @apiDescription Adds a new record into the [EntityName] table.  The entity is sent as a JSON document.
 * @apiExample This example adds a new "route".  Note:  Exclude the ID field as this field will be auto-generated.  The newly generated id is part of the returned JSON payload. :
 * http://localhost:8080/api/entity/route
 *
 *     body:
 *     {
 *       "routeName": "New Route"
 *       "routeDescription": "New Route Description"
 *     }
 *
 * @apiSuccessExample {json} Success-Response:
 *      HTTP/1.1 200 OK
 *      [{
 *              "id": "15",
 *              "routeDescription": "New Route",
 *              "routeName": "New Route Description"
 *          }
 *      ]
 *
 */

/**
 * @api {delete} /api/entity/[EntityName] Delete an Entity
 * @apiName DeleteEntity
 * @apiGroup Entity
 *
 * @apiDescription Deletes a specific record in the [EntityName] table.  The entity is sent as a JSON document.
 * @apiExample This example deletes a "route" with an ID of 7.:
 * http://localhost:8080/api/entity/route
 *
 *     body:
 *     {
 *       "id": 7
 *     }
 */

/// APIEntity
/// \brief  API handler class for LCS database entities.
///
/// This class handles basic database fetch and CRUD operations (Create, Update and Delete).  See the API documentation for more information.
class APIEntity : public QObject
{
    Q_OBJECT
public:
    /// Contstructor
    explicit APIEntity(QObject *parent = nullptr);

    /// API "/api/entity/[EntityName]"
    /// Handles fetch, add, update and delete functions on a database table.  [EntityName] represents the target table.
    /// @param request APIRequest containing the url of the request.  Used by the fetch (get) operation as optional where clause entries.
    /// @param response APIResponse with the payload set to the data returned from the database.
    void handleClient(const APIRequest &request, APIResponse *response);

    /// Fetches an entity list from the database with optional where clause values.
    /// @param name QString containing the table name from which data is fetched.
    /// @param url QUrl containing optional field/value pairs used in the WHERE clause.
    QByteArray fetchEntity(const QString &name, const QUrl &url);

    /// Updates a specific row in the [name] table in the database.
    /// @param name QString containing the table name.
    /// @param jsonText QString containing the data to be updated.
    QByteArray saveEntity(const QString &name, const QString &jsonText);

    /// Adds a new row into the [name] table in the database.
    /// @param name QString containing the table name.
    /// @param jsonText QString containing the data to be added.
    QByteArray addEntity(const QString &name, const QString &jsonText);

    /// Deletes a specific row in the [name] table in the database.
    /// @param name QString containing the table name.
    /// @param jsonText QString containing key data for the row to be deleted.
    void deleteEntity(const QString &name, const QString &jsonText);

private:
    /// Builds the where clause.
    /// @param url QUrl containing field/value pairs used in the WHERE clause.
    QString buildWhere(const QUrl &url);
    /// Handles additional processing after a successful data modification transaction.
    /// @param entityName QString containing the name of the entity that changed.
    /// @param jsonData QByteArra containing the data of the entity that changed.
    void handleTriggers(const QString &entityName, const QByteArray &jsonData);

};

#endif // APIENTITY_H
