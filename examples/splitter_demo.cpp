#include <wxpex/scrolled.h>
#include <wxpex/splitter.h>
#include <wxpex/slider.h>
#include <wxpex/collapsible.h>
#include <wxpex/static_box.h>
#include <wxpex/list_view.h>
#include <wxpex/spin_control.h>
#include <wxpex/labeled_widget.h>
#include <wxpex/widget_names.h>
#include <pex/group.h>
#include <pex/list.h>
#include <jive/testing/gettys_words.h>
#include <jive/strings.h>
#include <cctype>


template<typename T>
struct PricingFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::markup, "markup"),
        fields::Field(&T::taxRate, "taxRate"),
        fields::Field(&T::shipping, "shipping"));
};


using CountRangeMaker = pex::MakeRange<size_t, pex::Limit<0>, pex::Limit<100>>;


template<template<typename> typename T>
struct PricingTemplate
{
    T<double> markup;
    T<double> taxRate;
    T<double> shipping;

    static constexpr auto fieldsTypeName =
        PricingFields<PricingTemplate>::fields;
};


struct Pricing: public PricingTemplate<pex::Identity>
{
    using Base = PricingTemplate<pex::Identity>;

    Pricing()
        :
        Base{
            0.25,
            0.08,
            3.00}
    {

    }

    static Pricing Default()
    {
        return {};
    }

    double GetSalePrice(double cost) const
    {
        return (cost * (1.0 + this->markup) * (1.0 + this->taxRate))
            + this->shipping;
    }
};


using PricingGroup =
    pex::Group<PricingFields, PricingTemplate, pex::PlainT<Pricing>>;

using PricingControl = typename PricingGroup::Control;


template<typename T>
struct SettingsFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::pricing, "pricing"),
        fields::Field(&T::count, "count"));
};


using CountRangeMaker = pex::MakeRange<size_t, pex::Limit<0>, pex::Limit<100>>;


template<template<typename> typename T>
struct SettingsTemplate
{
    T<PricingGroup> pricing;
    T<CountRangeMaker> count;

    static constexpr auto fieldsTypeName =
        SettingsFields<SettingsTemplate>::fields;
};


using SettingsGroup = pex::Group<SettingsFields, SettingsTemplate>;
using SettingsControl = typename SettingsGroup::Control;
using CountControl = decltype(SettingsControl::count);



class CountView: public wxpex::Collapsible
{
public:
    CountView(wxWindow *parent, CountControl countControl)
        :
        wxpex::Collapsible(parent, "Count")
    {
        auto countSlider = new wxpex::ValueSlider(
            this->GetPanel(),
            countControl,
            countControl.value);

        wxpex::RegisterWidgetName(countSlider, "countSlider");

        auto sizer = wxpex::LayoutItems(wxpex::verticalItems, countSlider);
        this->ConfigureTopSizer(std::move(sizer));
    }
};


class PricingView: public wxpex::Collapsible
{
public:
    PricingView(wxWindow *parent, PricingControl pricingControl)
        :
        wxpex::Collapsible(parent, "Pricing", wxBORDER_SIMPLE)
    {
        auto markup = wxpex::LabeledWidget(
            this->GetPanel(),
            "Markup (%)",
            wxpex::CreateField<2>(this->GetPanel(), pricingControl.markup));

        auto taxRate = wxpex::LabeledWidget(
            this->GetPanel(),
            "Tax Rate (%)",
            wxpex::CreateField<2>(this->GetPanel(), pricingControl.taxRate));

        auto shipping = wxpex::LabeledWidget(
            this->GetPanel(),
            "Shipping",
            wxpex::CreateField<2>(this->GetPanel(), pricingControl.shipping));

        auto sizer = wxpex::LayoutLabeled(
            wxpex::LayoutOptions{},
            markup,
            taxRate,
            shipping);

        this->ConfigureTopSizer(std::move(sizer));
    }
};


class SettingsView: public wxpex::StaticBox
{
public:
    SettingsView(wxWindow *parent, SettingsControl settingsControl)
        :
        wxpex::StaticBox(parent, "Settings")
    {
        auto pricing = new PricingView(this, settingsControl.pricing);

        wxpex::RegisterWidgetName(pricing, "pricing");

        auto countSpinner =
            new wxpex::SpinControl(this, settingsControl.count, 1);

        wxpex::RegisterWidgetName(countSpinner, "countSpinner");

        auto count = new CountView(this->GetPanel(), settingsControl.count);

        wxpex::RegisterWidgetName(count, "count");

        count->Collapse(true);

        auto sizer = wxpex::LayoutItems(
            wxpex::verticalItems,
            pricing,
            countSpinner,
            count);

        this->ConfigureSizer(std::move(sizer));
    }
};


template<typename T>
struct BookFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::title, "title"),
        fields::Field(&T::author, "author"),
        fields::Field(&T::cost, "cost"),
        fields::Field(&T::sale, "sale"));
};


template<template<typename> typename T>
struct BookTemplate
{
    T<std::string> title;
    T<std::string> author;
    T<double> cost;
    T<double> sale;

    static constexpr auto fieldsTypeName =
        BookFields<BookTemplate>::fields;
};


using BookGroup = pex::Group<BookFields, BookTemplate>;
using BookControl = typename BookGroup::Control;


class BookView: public wxpex::StaticBox
{
public:
    BookView(wxWindow *parent, BookControl bookControl)
        :
        wxpex::StaticBox(parent, "Book")
    {
        auto title = wxpex::LabeledWidget(
            this->GetPanel(),
            "Title",
            wxpex::MakeWidget<wxpex::Field>(
                "Title",
                this->GetPanel(),
                bookControl.title));

        auto author = wxpex::LabeledWidget(
            this->GetPanel(),
            "Author",
            wxpex::MakeWidget<wxpex::Field>(
                "Author",
                this->GetPanel(),
                bookControl.author));

        auto cost = wxpex::LabeledWidget(
            this->GetPanel(),
            "Cost",
            wxpex::CreateField<2>(this->GetPanel(), bookControl.cost));

        wxpex::RegisterWidgetName(cost.GetWidget(), "cost");

        auto sale = wxpex::LabeledWidget(
            this->GetPanel(),
            "Sale",
            wxpex::CreateField<2>(this->GetPanel(), bookControl.sale));

        wxpex::RegisterWidgetName(sale.GetWidget(), "sale");

        auto sizer = wxpex::LayoutLabeled(
            wxpex::LayoutOptions{},
            title,
            author,
            cost,
            sale);

        this->ConfigureSizer(std::move(sizer));
    }
};


template<typename T>
struct BookStoreFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::settings, "settings"),
        fields::Field(&T::books, "books"));
};


using BookListMaker = pex::List<BookGroup, 0>;


template<template<typename> typename T>
struct BookStoreTemplate
{
    T<SettingsGroup> settings;
    T<BookListMaker> books;

    static constexpr auto fieldsTypeName =
        BookStoreFields<BookStoreTemplate>::fields;
};



class AuthorGenerator
{
public:
    static const inline std::vector<std::string> firstNames = {
        "George",
        "Charles",
        "Stephen",
        "J. K.",
        "James",
        "Agatha",
        "William",
        "Jane",
        "Franz"};

    static const inline std::vector<std::string> lastNames = {
        "Orwell",
        "Dickens",
        "King",
        "Rowling",
        "Joyce",
        "Christie",
        "Shakespeare",
        "Austen",
        "Kafka"};

    AuthorGenerator(size_t seed)
        :
        generator_(seed),
        distribution_(0, firstNames.size() - 1)
    {

    }

    std::string MakeName()
    {
        auto first = firstNames.at(this->distribution_(this->generator_));
        auto second = lastNames.at(this->distribution_(this->generator_));

        return jive::strings::Join({first, second}, ' ');
    }


private:
    std::mt19937_64 generator_;
    std::uniform_int_distribution<size_t> distribution_;
};


class CostGenerator
{
public:
    CostGenerator(size_t seed)
        :
        generator_(seed),
        distribution_(100, 3000)
    {

    }

    double MakeCost()
    {
        auto costInPennies = this->distribution_(this->generator_);

        return static_cast<double>(costInPennies) / 100.0;
    }

private:
    std::mt19937_64 generator_;
    std::uniform_int_distribution<size_t> distribution_;
};


class TitleGenerator
{
public:
    TitleGenerator(size_t seed)
        :
        words_()
    {
        this->words_.Seed(seed);
    }

    std::string MakeTitle()
    {
        auto titleWords = this->words_.MakeUniqueVector(3);

        for (auto &word: titleWords)
        {
            word.at(0) = static_cast<char>(
                std::toupper(static_cast<unsigned char>(word.at(0))));
        }

        return jive::strings::Join(
            std::begin(titleWords),
            std::end(titleWords),
            ' ');
    }


private:
    RandomGettysWords words_;
};


using BookListControl = pex::ControlSelector<BookListMaker>;


struct BookStoreCustom
{
    template<typename Base>
    class Model: public Base
    {
    public:
        Model()
            :
            Base(),
            countEndpoint_(this, this->settings.count, &Model::OnCount_),
            pricingEndpoint_(this, this->settings.pricing, &Model::OnPricing_)
        {

        }

        void OnCount_(size_t value)
        {
            this->books.count.Set(value);

            TitleGenerator titles(42);
            AuthorGenerator authors(314159);
            CostGenerator costs(2718);
            auto pricing = this->settings.pricing.Get();

            for (auto &book: BookListControl(this->books))
            {
                book.title.Set(titles.MakeTitle());
                book.author.Set(authors.MakeName());
                book.cost.Set(costs.MakeCost());
                book.sale.Set(pricing.GetSalePrice(book.cost.Get()));
            }
        }

        void OnPricing_(const Pricing &pricing)
        {
            for (auto &book: BookListControl(this->books))
            {
                book.sale.Set(pricing.GetSalePrice(book.cost.Get()));
            }
        }

    private:
        using CountEndpoint = pex::Endpoint<Model, CountControl>;
        CountEndpoint countEndpoint_;

        using PricingEndpoint = pex::Endpoint<Model, PricingControl>;
        PricingEndpoint pricingEndpoint_;
    };
};


using BookStoreGroup =
    pex::Group<BookStoreFields, BookStoreTemplate, BookStoreCustom>;

using BookStoreModel = typename BookStoreGroup::Model;
using BookStoreControl = typename BookStoreGroup::Control;




class BookListView: public wxpex::ListView<BookListControl>
{
public:
    using Base = wxpex::ListView<BookListControl>;
    using ListItem = typename Base::ListItem;

    BookListView(wxWindow *parent, BookListControl books)
        :
        Base(parent, books)
    {

    }

protected:
    wxWindow * CreateView_(ListItem &itemControl, size_t) override
    {
        auto result = new BookView(this, itemControl);
        wxpex::RegisterWidgetName(result, "BookView");

        return result;
    }
};


class ScrolledBookListView: public wxpex::Scrolled
{
public:
    ScrolledBookListView(wxWindow *parent, BookListControl control)
        :
        wxpex::Scrolled(parent)
    {
        auto bookList = new BookListView(this, control);

        wxpex::RegisterWidgetName(bookList, "bookListView");

        this->ConfigureTopSizer(
            wxpex::verticalScrolled,
            wxpex::LayoutItems(
                wxpex::verticalItems,
                bookList));
    }
};


class BookStoreView: public wxFrame
{
public:
    BookStoreView(BookStoreControl bookStoreControl)
        :
        wxFrame(nullptr, wxID_ANY, "Book Store")
    {
        auto splitter = new wxpex::Splitter(this);
        auto settings = new SettingsView(splitter, bookStoreControl.settings);

        auto bookList =
            new ScrolledBookListView(splitter, bookStoreControl.books);

        splitter->SplitVerticallyLeft(settings, bookList);

        auto sizer = new wxBoxSizer(wxVERTICAL);
        sizer->Add(splitter, 1, wxEXPAND);

        this->SetSizer(sizer);

        wxpex::RegisterWidgetName(splitter, "splitter");
        wxpex::RegisterWidgetName(settings, "settings");
        wxpex::RegisterWidgetName(bookList, "bookList");
    }
};


class ExampleApp: public wxApp
{
public:
    ExampleApp()
        :
        bookStore_{}
    {

    }

    bool OnInit() override
    {
        BookStoreView *bookStoreView =
            new BookStoreView(BookStoreControl(this->bookStore_));

        wxpex::RegisterWidgetName(bookStoreView, "bookStoreView");

        bookStoreView->Show();

        return true;
    }

private:
    BookStoreModel bookStore_;
};


// Creates the main function for us, and initializes the app's run loop.
wxshimIMPLEMENT_APP_CONSOLE(ExampleApp)
