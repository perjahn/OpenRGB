#include <QFileDialog>
#include <QMessageBox>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>

#include "filesystem.h"
#include "OpenRGBPluginsPage.h"
#include "ui_OpenRGBPluginsPage.h"

void EnableClickCallbackFunction(void* this_ptr, void* entry_ptr)
{
    Ui::OpenRGBPluginsPage* this_page = (Ui::OpenRGBPluginsPage*)this_ptr;

    this_page->on_EnableButton_clicked((Ui::OpenRGBPluginsEntry*)entry_ptr);
}

Ui::OpenRGBPluginsPage::OpenRGBPluginsPage(PluginManager* plugin_manager_ptr, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OpenRGBPluginsPageUi)
{
    plugin_manager = plugin_manager_ptr;
    ui->setupUi(this);

    RefreshList();
}

Ui::OpenRGBPluginsPage::~OpenRGBPluginsPage()
{
    delete ui;
}

void Ui::OpenRGBPluginsPage::RefreshList()
{
    ui->PluginsList->clear();
    entries.clear();

    for(unsigned int plugin_idx = 0; plugin_idx < plugin_manager->ActivePlugins.size(); plugin_idx++)
    {
        OpenRGBPluginsEntry* entry = new OpenRGBPluginsEntry();

        /*---------------------------------------------------------*\
        | Fill in plugin information fields                         |
        \*---------------------------------------------------------*/
        entry->ui->NameValue->setText(QString::fromStdString(plugin_manager->ActivePlugins[plugin_idx].info.Name));
        entry->ui->DescriptionValue->setText(QString::fromStdString(plugin_manager->ActivePlugins[plugin_idx].info.Description));
        entry->ui->VersionValue->setText(QString::fromStdString(plugin_manager->ActivePlugins[plugin_idx].info.Version));
        entry->ui->CommitValue->setText(QString::fromStdString(plugin_manager->ActivePlugins[plugin_idx].info.Commit));
        entry->ui->URLValue->setText(QString::fromStdString(plugin_manager->ActivePlugins[plugin_idx].info.URL));

        /*---------------------------------------------------------*\
        | Fill in plugin icon                                       |
        \*---------------------------------------------------------*/
        QPixmap pixmap(QPixmap::fromImage(plugin_manager->ActivePlugins[plugin_idx].info.Icon));

        entry->ui->IconView->setPixmap(pixmap);
        entry->ui->IconView->setScaledContents(true);

        /*---------------------------------------------------------*\
        | Fill in plugin path                                       |
        \*---------------------------------------------------------*/
        entry->ui->PathValue->setText(QString::fromStdString(plugin_manager->ActivePlugins[plugin_idx].path));

        /*---------------------------------------------------------*\
        | Fill in plugin enabled status                             |
        \*---------------------------------------------------------*/
        entry->ui->EnabledCheckBox->setChecked((plugin_manager->ActivePlugins[plugin_idx].enabled));

        entry->RegisterEnableClickCallback(EnableClickCallbackFunction, this);

        /*---------------------------------------------------------*\
        | Add the entry to the plugin list                          |
        \*---------------------------------------------------------*/
        QListWidgetItem* item = new QListWidgetItem;

        item->setSizeHint(entry->sizeHint());

        ui->PluginsList->addItem(item);
        ui->PluginsList->setItemWidget(item, entry);

        entries.push_back(entry);
    }
}

void Ui::OpenRGBPluginsPage::on_InstallPluginButton_clicked()
{
    QString     install_file    = QFileDialog::getOpenFileName(this, "Install OpenRGB Plugin", "", "DLL Files (*.dll; *.dylib; *.so; *.so.*)");

    std::string from_path       = install_file.toStdString();
    std::string to_path         = ResourceManager::get()->GetConfigurationDirectory() + "plugins/";
    std::string to_file         = to_path + filesystem::path(from_path).filename().string();
    bool        match           = false;

    for(unsigned int plugin_idx = 0; plugin_idx < plugin_manager->ActivePlugins.size(); plugin_idx++)
    {
        if(to_file == plugin_manager->ActivePlugins[plugin_idx].path)
        {
            match = true;
            break;
        }
    }

    if(match == true)
    {
        QMessageBox::StandardButton reply;

        reply = QMessageBox::question(this, "Replace Plugin", "A plugin with this filename is already installed.  Are you sure you want to replace this plugin?", QMessageBox::Yes | QMessageBox::No);

        if(reply != QMessageBox::Yes)
        {
            return;
        }
    }

    try
    {
        filesystem::copy(from_path, to_path, filesystem::copy_options::update_existing);

        //TODO: Unregister the old plugin and load the new one if matched
        // For now, don't load the new plugin

        if(match == false)
        {
            plugin_manager->LoadPlugin(to_path + "/" + filesystem::path(from_path).filename().string());

            RefreshList();
        }
    }
    catch(std::exception& e)
    {

    }
}

void Ui::OpenRGBPluginsPage::on_RemovePluginButton_clicked()
{
    QMessageBox::StandardButton reply;

    reply = QMessageBox::question(this, "Remove Plugin", "Are you sure you want to remove this plugin?", QMessageBox::Yes | QMessageBox::No);

    if(reply != QMessageBox::Yes)
    {
        return;
    }

    int cur_row = ui->PluginsList->currentRow();

    if(cur_row < 0)
    {
        return;
    }

    QListWidgetItem* item = ui->PluginsList->takeItem(cur_row);

    ui->PluginsList->removeItemWidget(item);
    delete item;

    //TODO: Unregister the plugin from the plugin manager

    filesystem::remove(entries[cur_row]->ui->PathValue->text().toStdString());

    delete entries[cur_row];
    entries.erase(entries.begin() + cur_row);
}

void Ui::OpenRGBPluginsPage::on_EnableButton_clicked(OpenRGBPluginsEntry* entry)
{
    /*-----------------------------------------------------*\
    | Open plugin list and check if plugin is in the list   |
    \*-----------------------------------------------------*/
    json plugin_settings = ResourceManager::get()->GetSettingsManager()->GetSettings("Plugins");

    /*-----------------------------------------------------*\
    | Search the settings to find the correct index         |
    \*-----------------------------------------------------*/
    std::string     name        = "";
    std::string     description = "";
    bool            enabled     = true;
    bool            found       = false;
    unsigned int    plugin_ct   = 0;
    unsigned int    plugin_idx  = 0;

    std::string     entry_name  = entry->ui->NameValue->text().toStdString();
    std::string     entry_desc  = entry->ui->DescriptionValue->text().toStdString();

    if(entry->ui->EnabledCheckBox->isChecked())
    {
        enabled                 = true;
    }
    else
    {
        enabled                 = false;
    }

    if(plugin_settings.contains("plugins"))
    {
        plugin_ct = plugin_settings["plugins"].size();

        for(plugin_idx = 0; plugin_idx < plugin_settings["plugins"].size(); plugin_idx++)
        {
            if(plugin_settings["plugins"][plugin_idx].contains("name"))
            {
                name        = plugin_settings["plugins"][plugin_idx]["name"];
            }

            if(plugin_settings["plugins"][plugin_idx].contains("description"))
            {
                description = plugin_settings["plugins"][plugin_idx]["description"];
            }

            if((entry_name == name)
             &&(entry_desc == description))
            {
                found = true;
                break;
            }
        }
    }

    /*-----------------------------------------------------*\
    | If the plugin was not in the list, add it to the list |
    | and default it to enabled, then save the settings     |
    \*-----------------------------------------------------*/
    if(!found)
    {
        plugin_settings["plugins"][plugin_ct]["name"]           = entry_name;
        plugin_settings["plugins"][plugin_ct]["description"]    = entry_desc;
        plugin_settings["plugins"][plugin_ct]["enabled"]        = enabled;

        ResourceManager::get()->GetSettingsManager()->SetSettings("Plugins", plugin_settings);
        ResourceManager::get()->GetSettingsManager()->SaveSettings();
    }
    else
    {
        plugin_settings["plugins"][plugin_idx]["enabled"]       = enabled;
        ResourceManager::get()->GetSettingsManager()->SetSettings("Plugins", plugin_settings);
        ResourceManager::get()->GetSettingsManager()->SaveSettings();
    }
}
